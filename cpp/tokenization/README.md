# Модуль токенізації

Інтерфейс `TokenizerStrategy::tokenize(const std::string&) const` повертає
`std::vector<Token>`. `TokenizerFactory::create(TokenizerType)` створює одну з
трьох реальних стратегій: `WhitespaceTokenizer`, `WordPunctuationTokenizer`,
`RuleBasedTokenizer`. GUI та `Application` використовують цю фабрику без
вбудованих запасних алгоритмів. `MockTokenizer` залишений як тестова заглушка,
але фабрика його не повертає.

Позиція токена — порядковий індекс від 0, речення нумеруються від 1,
`tokenId = -1` до побудови словника. Пробільні токени пропускаються.
Неправильний UTF-8 та невідомий тип стратегії спричиняють `std::invalid_argument`.

## Налаштування RuleBased

`RuleDetection.cpp` містить впорядкований список функцій розпізнавання:

```cpp
static const std::vector<DetectionRule> detectors = {
    RuleDetectors::url,
    RuleDetectors::email,
    RuleDetectors::predefinedWord,
    RuleDetectors::hashtag,
    RuleDetectors::number,
    RuleDetectors::compoundWord,
    RuleDetectors::word,
};
```

Стратегія викликає їх по черзі та використовує перший збіг. Щоб додати правило,
достатньо написати функцію або лямбду й додати її до списку перед загальнішими
правилами. Щоб вимкнути правило, видаліть або закоментуйте його запис.
Конструктор і фабрика не приймають конфігурацію. Після зміни списку потрібна пересбірка.

Тип функції визначений у `TokenRule.h`:

```cpp
using DetectionRule = std::function<RuleMatch(const RuleContext&, std::size_t)>;
```

`RuleContext` надає початковий UTF-8 текст та декодовані Unicode-символи.
Позиція `start` і `RuleMatch::length` вимірюються в Unicode-символах.
`length = 0` означає відсутність збігу; успішне правило також задає тип токена.
Одного `bool` недостатньо: токенізатору потрібно знати довжину й тип знайденого фрагмента.

Наприклад, додайте цю лямбду перед іншими правилами для розпізнавання `@mention`:

```cpp
[](const RuleContext& context, std::size_t start) -> RuleMatch {
    const auto& chars = context.characters;
    if (chars[start].value != U'@') return {};
    auto end = start + 1;
    while (end < chars.size() &&
           tokenization_detail::classify(chars[end].value) == TokenType::Word) {
        ++end;
    }
    return end > start + 1
        ? RuleMatch{end - start, TokenType::Special}
        : RuleMatch{};
},
```

`RuleDetectors.h/.cpp` містять стандартні функції. Сам `RuleBasedTokenizer`
відповідає за обхід тексту, виклик списку правил і створення токенів, не містить
переліку конкретних детекторів. Якщо жодне правило не спрацювало, він створює
токен з одного символу. Некоректна довжина результату спричиняє `std::logic_error`.
Для правил із попереднім індексуванням є кеш діапазонів у `RuleContext`, окремий
для кожного документа. Email-детектор заповнює свій кеш лише при першому виклику.
## Словник винятків

`PredefinedWords.h` містить словник позначень, які потрібно зберігати одним токеном:

```cpp
static const std::vector<Entry> entries = {
    {U"C++", TokenType::Special},
    {U"C++/CLI", TokenType::Special},
    {U"C#", TokenType::Special},
    {U".NET", TokenType::Special},
};
```

Щоб додати виняток, достатньо додати запис із текстом і типом токена до словника
та перебудувати проєкт. Змінювати детектор або список правил не потрібно.
Префікс `U` задає Unicode-рядок, тому можна додавати й не ASCII-позначення.
Цей словник винятків не є ML-словником `Vocabulary` та не призначає числові ID.

Загальний детектор `predefinedWord` порівнює записи без урахування регістру через
Unicode simple lowercase, зберігає оригінальний текст і вибирає найдовший збіг:
`C++/CLI` має перевагу над `C++`, незалежно від порядку записів у словнику.
Перед та після збігу не повинно бути літер, цифр, комбінувальних знаків або `_`.
Тому `.NETCore` і `C++17` не розпізнаються як словникові винятки.
Пунктуація поруч із винятком залишається окремими токенами.

## Порядок правил

1. URL за префіксами (типово HTTP, HTTPS, www; регістр префікса неважливий).
2. ASCII email із непорожньою локальною частиною та доменом щонайменше з двох міток.
3. Винятки зі словника `PredefinedWords.h` та хештеги з літерами, цифрами й підкресленнями.
4. Числа зі знаком, десятковою частиною та експонентою: `-12.50`, `.5`, `1e-5`.
5. Unicode-слова з цифрами та внутрішніми апострофами й дефісами.
6. Звичайна класифікація решти символів через `utf8proc`.

| Вхід | Результат RuleBased |
| --- | --- |
| `Hello, world!` | `Hello`, `,`, `world`, `!` |
| `test@example.com` | один `Special` |
| `https://example.com.` | URL як `Special`, крапка окремо |
| `-12.50` / `1e-5` | один `Number` |
| `don't` / `п’ять` | один `Word` |
| `mother-in-law` / `COVID-19` | один `Word` |
| `C++` / `C#` / `.NET` / `#machine_learning` | один `Special` |

Зовнішні лапки та незбалансовані закривальні дужки не входять до URL;
збалансовані дужки всередині шляху зберігаються. Кінцева пунктуація відділяється.
Крапки в числах, URL та email не завершують речення. Послідовності `.`, `!`, `?`,
`…`, `。`, `！`, `？` завершують речення один раз. Групування пунктуації налаштовується
в `Application`; сама стратегія повертає пунктуаційні символи окремо.

Це лексичні правила, а не повна перевірка URL/email за стандартами. Не реалізовані
дати, винятки для скорочень, quoted email та Unicode email. Коми не є роздільниками
тисяч; знаки валют і відсотки залишаються окремими токенами. Китайський текст
розпізнається як послідовність Unicode-літер без словникового поділу на слова.

Для фіксованого набору правил прохід займає O(N) часу та O(N) пам'яті.
Email-діапазони індексуються окремим проходом, щоб уникнути повторного сканування
всього залишку тексту для кожного слова.

## Використання

```cpp
#include "tokenization/TokenizerFactory.h"
auto strategy = TokenizerFactory::create(TokenizerType::RuleBased);
auto tokens = strategy->tokenize("Visit https://example.com.");
```

## Збирання й тести

Модуль підтримує C++14; застосунок загалом використовує C++17.
Потрібні CMake 3.14+, Git та `utf8proc` v2.10.0, який CMake завантажує автоматично.

```sh
cmake -S cpp/tokenization -B build/tokenization-check -DBUILD_TESTING=ON
cmake --build build/tokenization-check --config Debug
ctest --test-dir build/tokenization-check -C Debug --output-on-failure
```

Тести також включаються до загального збирання проєкту.
