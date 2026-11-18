# Завантаження текстів

У GUI оберіть `Encoding`, потім натисніть `Open File`.
Підтримуються UTF-8, UTF-16 LE, UTF-16 BE та Windows-1251.
Кодування вибирається явно; зміна списку не перекодовує вже завантажені дані.
Для іншого кодування повторно відкрийте файл.

UTF-8 та UTF-16 можна завантажувати з BOM або без нього. Початковий BOM
видаляється. Якщо BOM суперечить вибраному кодуванню, завантаження завершується
помилкою з назвою файла. UTF-32, некоректні послідовності, невизначений байт
Windows-1251 0x98 і символ NUL відхиляються. Вміст зберігається в UTF-8;
пробіли, регістр та переноси змінює окремий етап препроцесингу.

Відкритий файл стає одним документом і проходить етапи побудови словника,
кодування та експорту. Його текст можна редагувати в полі вводу.
`Clear Text` очищує ввід, `Load Sample Corpus` завантажує приклад.
Помилка читання/декодування не замінює поточні дані.
Пакетне завантаження каталогів доступне через API нижче.

## API

```cpp
#include "preprocessing/TextLoader.h"

auto file = TextLoader::loadFile(std::filesystem::u8path(u8"текст.txt"),
                                 TextEncoding::Windows1251);
// file.text: UTF-8; file.filePath: шлях у UTF-8
auto files = TextLoader::loadDirectory(std::filesystem::u8path(u8"тексти"),
                                      TextEncoding::Utf16LE);
```

Методи `TextLoader` кидають виняток у разі помилки. Каталог повертається
повністю або не повертається зовсім.

Існуючі методи `Dataset` отримали останній необов'язковий параметр кодування:

```cpp
Dataset dataset;
dataset.loadDocumentFromFile(u8"текст.txt", false, nullptr, TextEncoding::Windows1251);
dataset.loadFromDirectory(u8"тексти", ".txt", false, nullptr, TextEncoding::Utf16BE);
```

Попередні виклики залишаються сумісними: типово використовується UTF-8.
Для відсутнього/недоступного файла `loadDocumentFromFile` повертає `false`;
для відсутнього каталогу `loadFromDirectory` повертає 0. Помилки декодування
кидають виняток і не додають частково завантажені документи.
Для токенізації передайте callback або використовуйте `Application`:

```cpp
PipelineConfig config;
config.textEncoding = TextEncoding::Windows1251;
config.tokenizerType = TokenizerType::RuleBased;
Application app(config);
app.runOnDirectory(u8"тексти");
```

Для інтеграції з інтерфейсом також доступний `MainWindow::loadTextFile(utf8Path)`.

Перевірки завантаження та інтеграції: CMake target / CTest `text_loading_tests`.
