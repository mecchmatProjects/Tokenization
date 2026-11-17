# Модуль нормалізації токенів

Окремий етап після токенізації та перед морфологією і побудовою `Vocabulary`.
Підтримує C++14, використовує `utf8proc` і детектори модуля `tokenization`.

## Використання

```cpp
#include "normalization/Normalizer.h"

NormalizationConfig config;
config.unicode = UnicodeNormalization::NFC;
config.removeStopWords = true;
config.replaceNumbers = true;
config.replaceUrls = true;
config.replaceEmails = true;

StopWordDictionary stopWords;
stopWords.loadFromFile("resources/stopwords/en.txt");
auto normalized = Normalizer().normalize(tokens, config, stopWords);
```

Налаштування і словник передаються ззовні. Вхідний список не змінюється,
повертається новий список. Типово всі перетворення вимкнені.

## Налаштування

| Поле | Типово | Дія |
| --- | --- | --- |
| `unicode` | `None` | Зберегти форму Unicode, привести до NFC або NFKC |
| `replaceNumbers` | `false` | Токени `Number` замінити на `<NUM>` |
| `replaceUrls` | `false` | Цілі URL замінити на `<URL>` |
| `replaceEmails` | `false` | Цілі email замінити на `<EMAIL>` |
| `removePunctuation` | `false` | Видалити токени `Punctuation` |
| `removeStopWords` | `false` | Видалити слова зі словника stop words |
| `stopWordsCaseSensitive` | `false` | Порівнювати stop words з урахуванням регістру |

Порядок: Unicode -> URL/email -> числа -> пунктуація -> stop words.
NFC об'єднує канонічно еквівалентні представлення. NFKC додатково перетворює
сумісні форми, наприклад лігатуру `ﬁ` на `fi`, тому її треба вмикати свідомо.
Lowercase та пробіли залишаються в препроцесорі, стемінг — окремим наступним етапом.

URL перевіряються раніше email: символ `@` у посиланні не робить його email.
Перевіряється весь токен, а не наявність символу чи підрядка. Маркери мають тип
`Special`, зберігають регістр і не видаляються як stop words. Повторне застосування
нормалізатора не змінює вже створені маркери. Константи зберігаються у `SpecialMarkers.h`.

Нормалізатор не відновлює конструкції, які токенізатор уже розбив. Наприклад, для
замін десяткових чисел, URL та email рекомендовано RuleBased. Якщо Whitespace
зберіг URL разом із кінцевою крапкою, такий токен не вважається точним збігом URL.

## Метадані

`position` і `sentenceNumber` зберігаються, після фільтрації в позиціях можливі
пропуски. ID токена скидається до `-1`, якщо змінилися його текст або тип.
Незмінені токени зберігають ID. Початковий текст документа зберігається у `rawText`.
Unicode-перетворення й порівняння без урахування регістру перевіряють UTF-8 і
спричиняють `std::invalid_argument` для некоректного вводу. Повністю вимкнені
перетворення повертають копію без додаткового декодування.

## Словник stop words

`StopWordDictionary::loadFromFile()` читає UTF-8: одне слово на рядок, порожні рядки
пропускаються, BOM першого рядка та пробільні символи на краях обробляються.
Дублікати об'єднуються. Внутрішні пробіли заборонені. Успішне завантаження замінює
словник; помилка залишає попередній набір незмінним. Відсутній або недоступний файл
спричиняє `std::runtime_error`, некоректний UTF-8 або формат — `std::invalid_argument`.

Слова можна додавати через `add()`. `contains()` виконує точний пошук початкового
рядка; Unicode-форму й регістр для видалення враховує сам `Normalizer`. Для словника
та токенів застосовується однакова Unicode-форма; порівняння без урахування регістру
використовує Unicode case folding і не змінює текст збережених токенів.

У `resources/stopwords/en.txt` та `uk.txt` наведено невеликі приклади наборів,
які можна доповнювати під конкретний корпус. Це не повні мовні списки.
Словник stop words не пов'язаний зі словником винятків `PredefinedWords`
або ML-словником `Vocabulary`.

## Інтеграція

`PipelineConfig::normalizationConfig` задає всі налаштування, а `stopWords` — словник.
Якщо нова конфігурація не задана, підтримуються старі перемикачі видалення пунктуації
та замін маркерами. Явна конфігурація має над ними пріоритет.

GUI містить вибір None/NFC/NFKC, незалежні заміни, перемикач stop words, чутливість
до регістру й редаговане поле зі словами через пробіли. Поле вводу stop words
не є шляхом до файлу; файли завантажуються через API `loadFromFile()`.
Початковий список у GUI — англійський приклад. Вимкнення lowercase у препроцесорі
дозволяє окремо досліджувати чутливість stop words до регістру.

Складність — у середньому O(N + S), де N — обсяг текстів токенів, S — обсяг
словника. Ключі словника готуються один раз за виклик `normalize()`.

## Збирання й тести

З кореня проєкту після налаштування CMake:

```sh
cmake --build build/preprocessing-nmake
ctest --test-dir build/preprocessing-nmake --output-on-failure
```

Окремо модуль:

```sh
cmake -S cpp/normalization -B build/normalization
cmake --build build/normalization --config Debug
ctest --test-dir build/normalization -C Debug --output-on-failure
```
