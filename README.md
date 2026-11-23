# Tokenization

Програма на C++ для токенізації тексту та підготовки даних до машинного навчання.

## Що потрібно встановити

- Компілятор із підтримкою C++17: MSVC на Windows або GCC/Clang на Linux.
- CMake 3.14+ та Git.

Бібліотеку **utf8proc v2.10.0** програма використовує для обробки Unicode. CMake завантажує її автоматично під час першого збирання, тому потрібен інтернет. Окремо встановлювати її не потрібно.

## Як зібрати та запустити

Команди виконуйте з папки проєкту. `example.txt` — ваш текстовий файл у UTF-8.

### Linux (Ubuntu / Debian)

```bash
sudo apt install build-essential cmake git
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/tokenizer --file example.txt
```

### Windows

Установіть Visual Studio або Build Tools із компонентами C++, CMake та Git. Для генератора Visual Studio:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\tokenizer.exe --file example.txt
```

Для відкриття графічного інтерфейсу запустіть програму без параметрів. Інтерфейс написаний через **Windows API** і працює лише на Windows.

На Linux передбачено консольний режим: він обробляє текст і показує статистику, час виконання та швидкість обробки для бенчмарків. Список параметрів: `--help`. Linux-збирання під час підготовки цього README не перевірялося.

## Стороння бібліотека

**utf8proc v2.10.0** використовується для читання символів UTF-8, визначення їхніх категорій (наприклад, літери чи пунктуація), зміни регістру та нормалізації Unicode.

Початкові автори **utf8proc** — **Jan Behrens та Public Software Group**. Серед подальших учасників — **Steven G. Johnson, Jiahao Chen, Tony Kelman і Jonas Fonseca**. Ці автори створювали сторонню бібліотеку, яку використовує проєкт.

utf8proc поширюється за ліцензією MIT; для даних Unicode діють окремі умови. [Ліцензії та авторські права](https://github.com/JuliaStrings/utf8proc/blob/v2.10.0/LICENSE.md).
