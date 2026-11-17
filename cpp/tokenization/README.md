# Модуль токенізації

Модуль містить стратегії розбиття тексту на токени. Наразі він ще не повністю
допрацьований, але базовий інтерфейс `TokenizerStrategy` вже сформовано:
метод `tokenize(const std::string& text) const` повертає `std::vector<Token>`.
Для створення стратегії передбачено `TokenizerFactory::create(TokenizerType)`.

Є реалізації `WhitespaceTokenizer` (розбиття за пробільними символами) та
`WordPunctuationTokenizer` (виділення слів, чисел і пунктуації). Остання
стратегія, `RuleBased`, яка має враховувати правила токенізації, ще потребує реалізації.

Фабрика повертає `WhitespaceTokenizer` для `Whitespace` та
`WordPunctuationTokenizer` для `WordPunctuation`.
Лише `RuleBased` поки повертає `MockTokenizer`, який ігнорує вхідний текст і
повертає `Hello`, `world`, `42`, `!`. GUI для RuleBased використовує вбудовану
реалізацію `Application` замість цієї заглушки.

Позиція токена — порядковий індекс від 0, номер речення починається з 1,
`tokenId = -1` до побудови словника. Невідомий тип спричиняє `std::invalid_argument`.
## Використання

```cpp
#include "tokenization/TokenizerFactory.h"

auto tokenizer = TokenizerFactory::create(TokenizerType::WordPunctuation);
auto tokens = tokenizer->tokenize("Довільний текст");
// Повертає токени вхідного тексту.
```

## Збирання

Потрібні C++17 або новіший, CMake 3.14+ та Git. Для наявних стратегій
використовується `utf8proc` v2.10.0; CMake завантажує залежність під час першого
налаштування. 
