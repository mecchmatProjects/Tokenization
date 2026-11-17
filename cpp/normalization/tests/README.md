# Тести нормалізатора

`NormalizerTests.cpp` підключено до CTest як `normalizer_tests`.
Перевіряє незалежні налаштування та їх поєднання, NFC/NFKC, UTF-8, маркери,
Unicode-порівняння stop words, завантаження словника, метадані й незмінність вводу.

Передачу налаштувань через `Application` та обробники GUI перевіряє також
`cpp/preprocessing/tests/ApplicationPreprocessingTests.cpp`.
