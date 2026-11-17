# Модуль попередньої обробки тексту

`Preprocessor` задає інтерфейс `process(const std::string&) const`.
`TextPreprocessor` обробляє UTF-8 та повертає новий рядок, не змінюючи вхідні дані.
Модуль не залежить від токенізатора; обидві бібліотеки використовують спільний
`utf8proc`, оголошений у `cmake/Dependencies.cmake`.

```cpp
#include "preprocessing/TextPreprocessor.h"

PreprocessingConfig config;
config.lowercase = true;
config.newlineMode = NewlineMode::ReplaceWithSpace;
TextPreprocessor preprocessor(config);
auto text = preprocessor.process(u8"  ПРИВІТ\r\n  світе!  ");
// "привіт світе!"
```

## Налаштування

| Поле | Типово | Поведінка |
| --- | --- | --- |
| `collapseHorizontalWhitespace` | `true` | Табуляції та Unicode-категорію Zs замінює одним ASCII-пробілом |
| `trimWhitespace` | `true` | Видаляє пробільні символи на краях усього тексту |
| `lowercase` | `false` | Виконує Unicode simple lowercase через `utf8proc_tolower` |
| `newlineMode` | `NormalizeToLf` | Керує обробкою переносів |

Режими переносів: `Preserve`, `NormalizeToLf`, `ReplaceWithSpace`.
Переноси: CR, LF, CRLF, VT, FF, NEL (U+0085), LS (U+2028), PS (U+2029).
CRLF при перетворенні вважається одним переносом. `Preserve` зберігає внутрішні
переноси, але `trimWhitespace` може видалити їх на краях. Після вимкнення всіх
перетворень коректний UTF-8 зберігається без змін, включно з вбудованим NUL.

Порядок операцій: читання та перевірка UTF-8, обробка переносів, стиснення
горизонтальних пробілів, обрізання країв, lowercase. Реалізація об'єднує операції
в один прохід, час O(N), додаткова пам'ять O(N) для результату.
Некоректний UTF-8 спричиняє `std::invalid_argument` з байтовою позицією помилки.

Lowercase не є повним case folding або мовно-залежним перетворенням регістру.
Unicode NFC/NFKC, stop words, маркери, видалення пунктуації та декодування інших
кодувань не належать до цього модуля. BOM обробляється завантажувачем.

## Інтеграція

`Application::preprocessText()` адаптує наявні параметри `PipelineConfig`:
`normalizeWhitespace=true` вмикає стиснення, обрізання та заміну переносів пробілами;
`false` зберігає всі пробільні символи. `convertToLowercase` керує Unicode lowercase.
Пунктуація видаляється лише після токенізації за типом токена.
Готові токени в `runOnDataset()` також можуть проходити Unicode lowercase.
`PipelineConfig::preprocessingConfig` дозволяє незалежно задати всі параметри та
має пріоритет над старими `normalizeWhitespace` і `convertToLowercase`.
GUI використовує цей параметр і передає весь текст як один документ до
`Application::tokenizeText()`. Перемикачі lowercase, стиснення пробілів, trim та
список режимів переносів безпосередньо задають налаштування модуля.
Поле вводу та таблиці використовують Unicode Win32 API з перетворенням UTF-16/UTF-8.
Для Whitespace та Basic фабрика повертає модульні токенізатори; RuleBased у GUI
тимчасово використовує вбудовану реалізацію `Application`, доки окремий клас не готовий.

## Збирання й тести

З кореня репозиторію (для багатоконфігураційних генераторів):

```sh
cmake -S . -B build/preprocessing-check -DBUILD_TESTING=ON
cmake --build build/preprocessing-check --config Debug
ctest --test-dir build/preprocessing-check -C Debug --output-on-failure
```

Модуль також можна зібрати окремо: замінити `-S .` на `-S cpp/preprocessing`.
Перший запуск CMake завантажує `utf8proc` v2.10.0 через Git.
