# Приклади словників stop words

`en.txt` та `uk.txt` — невеликі початкові набори, які можна доповнювати під свій корпус.
Це не повні мовні списки. Формат: UTF-8, одне слово на рядок.

Завантаження з коду:

```cpp
StopWordDictionary dictionary;
dictionary.loadFromFile("resources/stopwords/uk.txt");
```

Видалення stop words типово вимкнене. У GUI слова можна вводити безпосередньо
в поле Stop words через пробіли.
