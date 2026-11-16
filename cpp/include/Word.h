#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Частини мови (Тип слова)
enum class PartOfSpeech {
    Unknown,
    Noun,       // Іменник
    Verb,       // Дієслово
    Adjective,  // Прикметник
    Numeral     // Чисельник
};

// Рід
enum class Gender {
    None,
    Masculine,  // Чоловічий
    Feminine,   // Жіночий
    Neuter      // Середній
};

// Число
enum class GrammaticalNumber {
    None,
    Singular,   // Однина
    Plural      // Множина
};

// Відмінок
enum class GrammaticalCase {
    None,
    Nominative,     // Називний (хто? що?)
    Genitive,       // Родовий (кого? чого?)
    Dative,         // Давальний (кому? чому?)
    Accusative,     // Знахідний (кого? що?)
    Instrumental,   // Орудний (ким? чим?)
    Locative,       // Місцевий (на кому? на чому?)
    Vocative        // Кличний
};

// Базовий клас СЛОВО (Роль 2)
class Word {
protected:
    PartOfSpeech type = PartOfSpeech::Unknown;
    Gender gender = Gender::None;
    GrammaticalNumber number = GrammaticalNumber::Singular;
    int person = 0; // Особа (0 - не визначено, 1, 2, 3)
    GrammaticalCase wordCase = GrammaticalCase::Nominative;

    std::vector<std::string> prefixes; // від 0 до 3 префіксів
    std::vector<std::string> roots;    // від 1 до 2 коренів
    std::vector<std::string> suffixes; // від 0 до 3 суфіксів
    std::string ending;                // від 0 до 1 закінчення

public:
    Word();
    explicit Word(const std::string& rawWord,
        PartOfSpeech pos = PartOfSpeech::Unknown,
        Gender g = Gender::None,
        GrammaticalNumber num = GrammaticalNumber::Singular,
        int pers = 0,
        GrammaticalCase c = GrammaticalCase::Nominative);
    virtual ~Word() = default;

    // --- Установка та отримання граматичних властивостей слова ---
    void setProperties(PartOfSpeech pos, Gender g, GrammaticalNumber num, int pers, GrammaticalCase c);
    PartOfSpeech getType() const { return type; }
    Gender getGender() const { return gender; }
    GrammaticalNumber getNumber() const { return number; }
    int getPerson() const { return person; }
    GrammaticalCase getCase() const { return wordCase; }

    // --- Введення/виведення слова по частинах (морфемах) ---
    bool setPrefixes(const std::vector<std::string>& prefs); // 0..3
    bool setRoots(const std::vector<std::string>& rts);      // 1..2
    bool setSuffixes(const std::vector<std::string>& suffs); // 0..3
    void setEnding(const std::string& end);                  // 0..1

    const std::vector<std::string>& getPrefixes() const { return prefixes; }
    const std::vector<std::string>& getRoots() const { return roots; }
    const std::vector<std::string>& getSuffixes() const { return suffixes; }
    const std::string& getEnding() const { return ending; }

    // Автоматичний морфемний розбір слова (префікс, корінь, суфікс, закінчення)
    void autoParseMorphemes(const std::string& rawWord);

    // Зібрати повне слово з морфем
    std::string getFullWord() const;

    // Отримати корінь/основу слова (для стемінгу та лематизації в ML-конвеєрі)
    std::string getStem() const;
    std::string getLemma() const;

    // --- Введення/виведення частин слова з/в консоль та файл ---
    void inputMorphemesFromConsole();
    void printMorphemes(std::ostream& os = std::cout) const;

    void inputFromConsole();
    void outputToConsole(std::ostream& os = std::cout) const;

    bool saveToFile(const std::string& filePath) const;
    bool loadFromFile(const std::string& filePath);

    // --- Розбиття слова на склади (для переносу) та злиття складів ---
    std::vector<std::string> splitIntoSyllables() const;
    std::string getHyphenatedWord() const;

    static std::vector<std::string> splitWordIntoSyllables(const std::string& word);
    static std::string mergeSyllables(const std::vector<std::string>& syllables);

    // --- Зміна властивостей (переведення в інше число та відмінок) ---
    virtual void changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase);

    // --- Інтеграція з Application (Роль 1 та Роль 3) ---
    // Функція для передачі в Application::setMorphologyNormalizer(...)
    static std::string normalizeTokenText(const std::string& tokenText);

    // Допоміжні методи перетворення enum у текст
    static std::string posToString(PartOfSpeech pos);
    static std::string genderToString(Gender g);
    static std::string numberToString(GrammaticalNumber n);
    static std::string caseToString(GrammaticalCase c);
};

// ============================================================================
// Наслідники класу СЛОВО: ІМЕННИК, ДІЄСЛОВО, ПРИКМЕТНИК, ЧИСЕЛЬНИК
// ============================================================================

class Noun : public Word {
public:
    Noun();
    explicit Noun(const std::string& rawWord,
        Gender g = Gender::Masculine,
        GrammaticalNumber num = GrammaticalNumber::Singular,
        GrammaticalCase c = GrammaticalCase::Nominative);

    void changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) override;
};

class Verb : public Word {
public:
    Verb();
    explicit Verb(const std::string& rawWord,
        GrammaticalNumber num = GrammaticalNumber::Singular,
        int pers = 3,
        Gender g = Gender::None);

    void changePersonAndNumber(int newPerson, GrammaticalNumber newNumber);
    void changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) override;
};

class Adjective : public Word {
public:
    Adjective();
    explicit Adjective(const std::string& rawWord,
        Gender g = Gender::Masculine,
        GrammaticalNumber num = GrammaticalNumber::Singular,
        GrammaticalCase c = GrammaticalCase::Nominative);

    void changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) override;
};

class Numeral : public Word {
public:
    Numeral();
    explicit Numeral(const std::string& rawWord,
        GrammaticalNumber num = GrammaticalNumber::Singular,
        GrammaticalCase c = GrammaticalCase::Nominative);

    void changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) override;
};