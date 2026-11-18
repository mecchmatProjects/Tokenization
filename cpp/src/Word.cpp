#include "../include/Word.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace {

    // Розбиття UTF-8 рядка на масив окремих символів (коректна робота з кирилицею та ASCII)
    std::vector<std::string> utf8ToChars(const std::string& str) {
        std::vector<std::string> chars;
        size_t i = 0;
        while (i < str.size()) {
            unsigned char c = static_cast<unsigned char>(str[i]);
            size_t len = 1;
            if ((c & 0x80) == 0) {
                len = 1;
            }
            else if ((c & 0xE0) == 0xC0) {
                len = 2;
            }
            else if ((c & 0xF0) == 0xE0) {
                len = 3;
            }
            else if ((c & 0xF8) == 0xF0) {
                len = 4;
            }
            if (i + len > str.size()) len = 1;
            chars.push_back(str.substr(i, len));
            i += len;
        }
        return chars;
    }

    bool isUkrVowel(const std::string& ch) {
        static const std::vector<std::string> vowels = {
            "а", "е", "є", "и", "і", "ї", "о", "у", "ю", "я",
            "А", "Е", "Є", "И", "І", "Ї", "О", "У", "Ю", "Я",
            "a", "e", "i", "o", "u", "y", "A", "E", "I", "O", "U", "Y"
        };
        return std::find(vowels.begin(), vowels.end(), ch) != vowels.end();
    }

    bool isYot(const std::string& ch) {
        return ch == "й" || ch == "Й";
    }

    bool isSoftSign(const std::string& ch) {
        return ch == "ь" || ch == "Ь" || ch == "'" || ch == "’";
    }

    bool isLetterChar(const std::string& ch) {
        if (ch.empty()) return false;
        if (ch.size() == 1) {
            return std::isalpha(static_cast<unsigned char>(ch[0])) != 0;
        }
        return true; // Кириличні UTF-8 символи
    }

    bool isConsonantUnit(const std::string& unit) {
        auto chars = utf8ToChars(unit);
        if (chars.empty()) return false;
        return isLetterChar(chars[0]) && !isUkrVowel(chars[0]);
    }

    bool isVowelUnit(const std::string& unit) {
        auto chars = utf8ToChars(unit);
        if (chars.empty()) return false;
        return isUkrVowel(chars[0]);
    }

    // Згрупувати букви за правилами Syllabls.md:
    // - буква 'й' розглядається разом з попередньою голосною як єдине ціле
    // - буква 'ь' (та апостроф) разом з попередньою приголосною як єдине ціле
    std::vector<std::string> buildPhoneticUnits(const std::string& word) {
        std::vector<std::string> rawChars = utf8ToChars(word);
        std::vector<std::string> units;

        for (const auto& ch : rawChars) {
            if (!units.empty() && isYot(ch) && isVowelUnit(units.back())) {
                units.back() += ch;
            }
            else if (!units.empty() && isSoftSign(ch) && isConsonantUnit(units.back())) {
                units.back() += ch;
            }
            else {
                units.push_back(ch);
            }
        }
        return units;
    }

    bool containsVowelInRange(const std::vector<std::string>& units, size_t startIdx, size_t endIdx) {
        for (size_t i = startIdx; i < endIdx && i < units.size(); ++i) {
            if (isVowelUnit(units[i])) return true;
        }
        return false;
    }

    bool startsWithBytes(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }

    bool endsWithBytes(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    // Розбиття однієї частини слова (одного кроку) за правилами 1, 2, 3 з Syllabls.md
    std::vector<std::string> splitIntoSyllablesRecursive(const std::string& segment) {
        std::vector<std::string> units = buildPhoneticUnits(segment);
        size_t n = units.size();

        // Якщо у фрагменті менше 2 голосних — це вже один склад
        size_t vowelCount = 0;
        for (const auto& u : units) {
            if (isVowelUnit(u)) ++vowelCount;
        }
        if (vowelCount <= 1 || n <= 1) {
            return { segment };
        }

        // ПРАВИЛО 3 (попередній крок із Syllabls.md):
        // Спробувати виділити приставку, що містить голосну, якщо після неї є ще голосна
        static const std::vector<std::string> vowelPrefixes = {
            "пере", "перед", "через", "між", "понад", "після",
            "при", "пре", "про", "пра", "під", "підо", "над", "надо",
            "від", "відо", "роз", "розі", "без", "безо",
            "за", "на", "по", "до", "ви", "у", "о"
        };

        for (const auto& pref : vowelPrefixes) {
            if (startsWithBytes(segment, pref) && segment.size() > pref.size()) {
                std::string rest = segment.substr(pref.size());
                auto restUnits = buildPhoneticUnits(rest);
                if (containsVowelInRange(restUnits, 0, restUnits.size())) {
                    // Якщо префікс закінчується на приголосну, а далі йде голосна (напр. ро-зіграти),
                    // даємо звичайним правилам обробити межу, інакше відділяємо префікс
                    auto prefUnits = buildPhoneticUnits(pref);
                    if (prefUnits.size() > 1 && isVowelUnit(prefUnits.back())) {
                        std::vector<std::string> result = { pref };
                        auto tail = splitIntoSyllablesRecursive(rest);
                        result.insert(result.end(), tail.begin(), tail.end());
                        return result;
                    }
                }
            }
        }

        // ПРАВИЛО 1 (Syllabls.md):
        // Дві підряд голосні можна розділити, якщо першій з них передує приголосна,
        // а за другою йде хоча б одна буква ('й' вже об'єднано з попередньою голосною).
        for (size_t i = 1; i + 2 < n; ++i) {
            if (isConsonantUnit(units[i - 1]) && isVowelUnit(units[i]) && isVowelUnit(units[i + 1])) {
                std::string firstPart;
                for (size_t k = 0; k <= i; ++k) firstPart += units[k];
                std::string secondPart;
                for (size_t k = i + 1; k < n; ++k) secondPart += units[k];

                std::vector<std::string> result = { firstPart };
                auto tail = splitIntoSyllablesRecursive(secondPart);
                result.insert(result.end(), tail.begin(), tail.end());
                return result;
            }
        }

        // ПРАВИЛО 2 (Syllabls.md):
        // Дві підряд приголосні можна розділити, якщо першій з них передує голосна,
        // а в тій частині слова, яка йде за другою приголосною, є хоча б одна голосна
        // ('ь' вже об'єднано з попередньою приголосною).
        for (size_t i = 1; i + 1 < n; ++i) {
            if (isVowelUnit(units[i - 1]) && isConsonantUnit(units[i]) && isConsonantUnit(units[i + 1]) &&
                containsVowelInRange(units, i + 2, n)) {
                std::string firstPart;
                for (size_t k = 0; k <= i; ++k) firstPart += units[k];
                std::string secondPart;
                for (size_t k = i + 1; k < n; ++k) secondPart += units[k];

                std::vector<std::string> result = { firstPart };
                auto tail = splitIntoSyllablesRecursive(secondPart);
                result.insert(result.end(), tail.begin(), tail.end());
                return result;
            }
        }

        // ПРАВИЛО 3 (основна частина із Syllabls.md):
        // Якщо не вдалося застосувати п. 1 і 2, розбити слово так, щоб перша частина
        // містила більш ніж одну букву і закінчувалася на голосну, а друга містила хоча б одну голосну.
        for (size_t i = 0; i + 1 < n; ++i) {
            std::string firstPart;
            for (size_t k = 0; k <= i; ++k) firstPart += units[k];
            auto firstChars = utf8ToChars(firstPart);

            if (firstChars.size() > 1 && isVowelUnit(units[i]) && containsVowelInRange(units, i + 1, n)) {
                std::string secondPart;
                for (size_t k = i + 1; k < n; ++k) secondPart += units[k];

                std::vector<std::string> result = { firstPart };
                auto tail = splitIntoSyllablesRecursive(secondPart);
                result.insert(result.end(), tail.begin(), tail.end());
                return result;
            }
        }

        return { segment };
    }

} // namespace

// ============================================================================
// Реалізація базового класу Word
// ============================================================================

Word::Word() : roots({ "слово" }) {}

Word::Word(const std::string& rawWord,
    PartOfSpeech pos,
    Gender g,
    GrammaticalNumber num,
    int pers,
    GrammaticalCase c)
    : type(pos), gender(g), number(num), person(pers), wordCase(c) {
    autoParseMorphemes(rawWord);
}

void Word::setProperties(PartOfSpeech pos, Gender g, GrammaticalNumber num, int pers, GrammaticalCase c) {
    type = pos;
    gender = g;
    number = num;
    person = pers;
    wordCase = c;
}

bool Word::setPrefixes(const std::vector<std::string>& prefs) {
    if (prefs.size() > 3) return false; // Від 0 до 3 префіксів
    prefixes = prefs;
    return true;
}

bool Word::setRoots(const std::vector<std::string>& rts) {
    if (rts.empty() || rts.size() > 2) return false; // Від 1 до 2 коренів
    roots = rts;
    return true;
}

bool Word::setSuffixes(const std::vector<std::string>& suffs) {
    if (suffs.size() > 3) return false; // Від 0 до 3 суфіксів
    suffixes = suffs;
    return true;
}

void Word::setEnding(const std::string& end) {
    ending = end; // Від 0 до 1 закінчення
}

void Word::autoParseMorphemes(const std::string& rawWord) {
    prefixes.clear();
    roots.clear();
    suffixes.clear();
    ending.clear();

    if (rawWord.empty()) {
        roots.push_back("");
        return;
    }

    std::string remaining = rawWord;

    // 1. Виділення префіксів (до 3 штук, укр + англ)
    static const std::vector<std::string> knownPrefixes = {
        "пере", "пред", "при", "пре", "про", "під", "над", "від", "роз", "без",
        "між", "за", "на", "по", "до", "ви", "спів", "не", "най",
        "inter", "trans", "under", "over", "pre", "pro", "anti", "sub", "un", "re", "de", "dis", "in"
    };

    for (int pCount = 0; pCount < 3; ++pCount) {
        bool matched = false;
        for (const auto& pref : knownPrefixes) {
            if (startsWithBytes(remaining, pref)) {
                auto remChars = utf8ToChars(remaining.substr(pref.size()));
                if (remChars.size() >= 3) { // Залишаємо місце для кореня
                    prefixes.push_back(pref);
                    remaining = remaining.substr(pref.size());
                    matched = true;
                    break;
                }
            }
        }
        if (!matched) break;
    }

    // 2. Виділення закінчення (0..1)
    static const std::vector<std::string> knownEndings = {
        "ами", "ями", "ові", "еві", "єві", "има", "ої", "ій", "им", "ім",
        "их", "їх", "ий", "ій", "ая", "яя", "еє", "єє", "ого", "ього", "ому", "ьому",
        "ть", "ти", "мо", "те", "уть", "ють", "ать", "ять", "еш", "єш", "иш", "їш",
        "ам", "ям", "ах", "ях", "ою", "ею", "єю", "ів", "їв", "ей",
        "а", "я", "о", "е", "є", "и", "і", "ї", "у", "ю",
        "ing", "ed", "es", "s"
    };

    for (const auto& endStr : knownEndings) {
        if (endsWithBytes(remaining, endStr)) {
            std::string stemCandidate = remaining.substr(0, remaining.size() - endStr.size());
            if (utf8ToChars(stemCandidate).size() >= 2) {
                ending = endStr;
                remaining = stemCandidate;
                break;
            }
        }
    }

    // 3. Виділення суфіксів (0..3)
    static const std::vector<std::string> knownSuffixes = {
        "уванн", "юванн", "льник", "ість", "ств", "ськ", "цьк", "зьк",
        "ник", "івн", "ач", "яч", "ець", "еньк", "оньк", "ува", "юва",
        "к", "н", "ов", "ев", "єв", "ин", "ін", "ар", "яр",
        "ation", "ition", "ment", "ness", "able", "ible", "ful", "less", "ize", "ise", "er", "or", "ly"
    };

    std::vector<std::string> extractedSuffixes;
    for (int sCount = 0; sCount < 3; ++sCount) {
        bool matched = false;
        for (const auto& suff : knownSuffixes) {
            if (endsWithBytes(remaining, suff)) {
                std::string rootCandidate = remaining.substr(0, remaining.size() - suff.size());
                if (utf8ToChars(rootCandidate).size() >= 2) {
                    extractedSuffixes.push_back(suff);
                    remaining = rootCandidate;
                    matched = true;
                    break;
                }
            }
        }
        if (!matched) break;
    }
    std::reverse(extractedSuffixes.begin(), extractedSuffixes.end());
    suffixes = extractedSuffixes;

    // 4. Виділення коренів (1..2): перевірка на складне слово зі сполучною голосною або дефісом
    size_t dashPos = remaining.find('-');
    if (dashPos != std::string::npos && dashPos > 0 && dashPos + 1 < remaining.size()) {
        roots.push_back(remaining.substr(0, dashPos));
        roots.push_back(remaining.substr(dashPos + 1));
    }
    else {
        roots.push_back(remaining);
    }
}

std::string Word::getFullWord() const {
    std::string result;
    for (const auto& p : prefixes) result += p;
    for (size_t i = 0; i < roots.size(); ++i) {
        result += roots[i];
    }
    for (const auto& s : suffixes) result += s;
    result += ending;
    return result;
}

std::string Word::getStem() const {
    // Для стемінгу повертаємо головний корінь (або з'єднані корені без афіксів)
    if (!roots.empty() && !roots[0].empty()) {
        std::string core;
        for (const auto& r : roots) core += r;
        return core;
    }
    return getFullWord();
}

std::string Word::getLemma() const {
    std::string base;
    for (const auto& p : prefixes) base += p;
    for (const auto& r : roots) base += r;
    for (const auto& s : suffixes) base += s;

    if (type == PartOfSpeech::Verb) {
        return base + "ти";
    }
    if (type == PartOfSpeech::Adjective) {
        if (gender == Gender::Feminine) return base + "а";
        if (gender == Gender::Neuter) return base + "е";
        return base + "ий";
    }
    if (type == PartOfSpeech::Noun) {
        if (gender == Gender::Feminine && !ending.empty()) return base + "а";
        if (gender == Gender::Neuter && !ending.empty()) return base + "о";
        return base;
    }
    return base;
}

void Word::inputMorphemesFromConsole() {
    int pCount = 0, rCount = 1, sCount = 0, hasEnd = 0;

    std::cout << "Кількість префіксів (0-3): ";
    std::cin >> pCount;
    pCount = std::clamp(pCount, 0, 3);
    prefixes.resize(pCount);
    for (int i = 0; i < pCount; ++i) {
        std::cout << "  Префікс " << (i + 1) << ": ";
        std::cin >> prefixes[i];
    }

    std::cout << "Кількість коренів (1-2): ";
    std::cin >> rCount;
    rCount = std::clamp(rCount, 1, 2);
    roots.resize(rCount);
    for (int i = 0; i < rCount; ++i) {
        std::cout << "  Корінь " << (i + 1) << ": ";
        std::cin >> roots[i];
    }

    std::cout << "Кількість суфіксів (0-3): ";
    std::cin >> sCount;
    sCount = std::clamp(sCount, 0, 3);
    suffixes.resize(sCount);
    for (int i = 0; i < sCount; ++i) {
        std::cout << "  Суфікс " << (i + 1) << ": ";
        std::cin >> suffixes[i];
    }

    std::cout << "Чи є закінчення? (0 - нульове, 1 - є): ";
    std::cin >> hasEnd;
    if (hasEnd == 1) {
        std::cout << "  Закінчення: ";
        std::cin >> ending;
    }
    else {
        ending.clear();
    }
}

void Word::printMorphemes(std::ostream& os) const {
    os << "Слово: " << getFullWord() << "\n";
    os << "  Префікси (" << prefixes.size() << "): ";
    if (prefixes.empty()) os << "-";
    for (const auto& p : prefixes) os << "[" << p << "-] ";
    os << "\n  Корені (" << roots.size() << "): ";
    for (const auto& r : roots) os << "[" << r << "] ";
    os << "\n  Суфікси (" << suffixes.size() << "): ";
    if (suffixes.empty()) os << "-";
    for (const auto& s : suffixes) os << "[-" << s << "] ";
    os << "\n  Закінчення: " << (ending.empty() ? "[нульове]" : "[" + ending + "]") << "\n";
}

void Word::inputFromConsole() {
    std::string raw;
    std::cout << "Введіть слово: ";
    std::cin >> raw;
    autoParseMorphemes(raw);
}

void Word::outputToConsole(std::ostream& os) const {
    os << "=== Інформація про слово ===\n";
    os << "Повне слово: " << getFullWord() << "\n";
    os << "Поділ на склади: " << getHyphenatedWord() << "\n";
    os << "Частина мови: " << posToString(type)
        << " | Рід: " << genderToString(gender)
        << " | Число: " << numberToString(number)
        << " | Особа: " << person
        << " | Відмінок: " << caseToString(wordCase) << "\n";
    printMorphemes(os);
}

bool Word::saveToFile(const std::string& filePath) const {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << static_cast<int>(type) << " "
        << static_cast<int>(gender) << " "
        << static_cast<int>(number) << " "
        << person << " "
        << static_cast<int>(wordCase) << "\n";

    out << prefixes.size();
    for (const auto& p : prefixes) out << " " << p;
    out << "\n";

    out << roots.size();
    for (const auto& r : roots) out << " " << r;
    out << "\n";

    out << suffixes.size();
    for (const auto& s : suffixes) out << " " << s;
    out << "\n";

    out << (ending.empty() ? 0 : 1) << (ending.empty() ? "" : " " + ending) << "\n";
    return true;
}

bool Word::loadFromFile(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;

    int t = 0, g = 0, n = 0, p = 0, c = 0;
    if (!(in >> t >> g >> n >> p >> c)) return false;
    type = static_cast<PartOfSpeech>(t);
    gender = static_cast<Gender>(g);
    number = static_cast<GrammaticalNumber>(n);
    person = p;
    wordCase = static_cast<GrammaticalCase>(c);

    size_t count = 0;
    in >> count;
    prefixes.resize(std::min<size_t>(count, 3));
    for (size_t i = 0; i < prefixes.size(); ++i) in >> prefixes[i];

    in >> count;
    roots.resize(std::clamp<size_t>(count, 1, 2));
    for (size_t i = 0; i < roots.size(); ++i) in >> roots[i];

    in >> count;
    suffixes.resize(std::min<size_t>(count, 3));
    for (size_t i = 0; i < suffixes.size(); ++i) in >> suffixes[i];

    int hasEnd = 0;
    in >> hasEnd;
    if (hasEnd == 1) {
        in >> ending;
    }
    else {
        ending.clear();
    }
    return true;
}

std::vector<std::string> Word::splitIntoSyllables() const {
    return splitWordIntoSyllables(getFullWord());
}

std::string Word::getHyphenatedWord() const {
    auto sylls = splitIntoSyllables();
    std::string res;
    for (size_t i = 0; i < sylls.size(); ++i) {
        res += sylls[i];
        if (i + 1 < sylls.size()) res += "-";
    }
    return res;
}

std::vector<std::string> Word::splitWordIntoSyllables(const std::string& word) {
    if (word.empty()) return {};
    return splitIntoSyllablesRecursive(word);
}

std::string Word::mergeSyllables(const std::vector<std::string>& syllables) {
    std::string merged;
    for (const auto& s : syllables) {
        merged += s;
    }
    return merged;
}

void Word::changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) {
    number = newNumber;
    wordCase = newCase;
}

std::string Word::normalizeTokenText(const std::string& tokenText) {
    if (tokenText.empty()) return tokenText;
    Word w(tokenText);
    std::string stem = w.getStem();
    return stem.empty() ? tokenText : stem;
}

std::string Word::posToString(PartOfSpeech pos) {
    switch (pos) {
    case PartOfSpeech::Noun:      return "Іменник";
    case PartOfSpeech::Verb:      return "Дієслово";
    case PartOfSpeech::Adjective: return "Прикметник";
    case PartOfSpeech::Numeral:   return "Чисельник";
    default:                      return "Невідомо";
    }
}

std::string Word::genderToString(Gender g) {
    switch (g) {
    case Gender::Masculine: return "Чоловічий";
    case Gender::Feminine:  return "Жіночий";
    case Gender::Neuter:    return "Середній";
    default:                return "Не визначено";
    }
}

std::string Word::numberToString(GrammaticalNumber n) {
    switch (n) {
    case GrammaticalNumber::Singular: return "Однина";
    case GrammaticalNumber::Plural:   return "Множина";
    default:                          return "Не визначено";
    }
}

std::string Word::caseToString(GrammaticalCase c) {
    switch (c) {
    case GrammaticalCase::Nominative:   return "Називний";
    case GrammaticalCase::Genitive:     return "Родовий";
    case GrammaticalCase::Dative:       return "Давальний";
    case GrammaticalCase::Accusative:   return "Знахідний";
    case GrammaticalCase::Instrumental: return "Орудний";
    case GrammaticalCase::Locative:     return "Місцевий";
    case GrammaticalCase::Vocative:     return "Кличний";
    default:                            return "Не визначено";
    }
}

// ============================================================================
// Реалізація класів-наслідників (ІМЕННИК, ДІЄСЛОВО, ПРИКМЕТНИК, ЧИСЕЛЬНИК)
// ============================================================================

Noun::Noun() {
    type = PartOfSpeech::Noun;
}

Noun::Noun(const std::string& rawWord, Gender g, GrammaticalNumber num, GrammaticalCase c)
    : Word(rawWord, PartOfSpeech::Noun, g, num, 3, c) {
}

void Noun::changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) {
    number = newNumber;
    wordCase = newCase;

    if (newNumber == GrammaticalNumber::Plural) {
        switch (newCase) {
        case GrammaticalCase::Nominative:   ending = "и"; break;
        case GrammaticalCase::Genitive:     ending = (gender == Gender::Feminine) ? "" : "ів"; break;
        case GrammaticalCase::Dative:       ending = "ам"; break;
        case GrammaticalCase::Accusative:   ending = "и"; break;
        case GrammaticalCase::Instrumental: ending = "ами"; break;
        case GrammaticalCase::Locative:     ending = "ах"; break;
        case GrammaticalCase::Vocative:     ending = "и"; break;
        default: break;
        }
    }
    else {
        if (gender == Gender::Feminine) {
            switch (newCase) {
            case GrammaticalCase::Nominative:   ending = "а"; break;
            case GrammaticalCase::Genitive:     ending = "и"; break;
            case GrammaticalCase::Dative:       ending = "і"; break;
            case GrammaticalCase::Accusative:   ending = "у"; break;
            case GrammaticalCase::Instrumental: ending = "ою"; break;
            case GrammaticalCase::Locative:     ending = "і"; break;
            case GrammaticalCase::Vocative:     ending = "о"; break;
            default: break;
            }
        }
        else if (gender == Gender::Neuter) {
            switch (newCase) {
            case GrammaticalCase::Nominative:   ending = "о"; break;
            case GrammaticalCase::Genitive:     ending = "а"; break;
            case GrammaticalCase::Dative:       ending = "у"; break;
            case GrammaticalCase::Accusative:   ending = "о"; break;
            case GrammaticalCase::Instrumental: ending = "ом"; break;
            case GrammaticalCase::Locative:     ending = "і"; break;
            case GrammaticalCase::Vocative:     ending = "о"; break;
            default: break;
            }
        }
        else { // Masculine
            switch (newCase) {
            case GrammaticalCase::Nominative:   ending = ""; break;
            case GrammaticalCase::Genitive:     ending = "а"; break;
            case GrammaticalCase::Dative:       ending = "ові"; break;
            case GrammaticalCase::Accusative:   ending = "а"; break;
            case GrammaticalCase::Instrumental: ending = "ом"; break;
            case GrammaticalCase::Locative:     ending = "ові"; break;
            case GrammaticalCase::Vocative:     ending = "е"; break;
            default: break;
            }
        }
    }
}

Verb::Verb() {
    type = PartOfSpeech::Verb;
}

Verb::Verb(const std::string& rawWord, GrammaticalNumber num, int pers, Gender g)
    : Word(rawWord, PartOfSpeech::Verb, g, num, pers, GrammaticalCase::None) {
}

void Verb::changePersonAndNumber(int newPerson, GrammaticalNumber newNumber) {
    person = std::clamp(newPerson, 1, 3);
    number = newNumber;

    if (newNumber == GrammaticalNumber::Singular) {
        if (person == 1) ending = "ю";
        else if (person == 2) ending = "єш";
        else ending = "є";
    }
    else {
        if (person == 1) ending = "ємо";
        else if (person == 2) ending = "єте";
        else ending = "ють";
    }
}

void Verb::changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase /*newCase*/) {
    changePersonAndNumber(person > 0 ? person : 3, newNumber);
}

Adjective::Adjective() {
    type = PartOfSpeech::Adjective;
}

Adjective::Adjective(const std::string& rawWord, Gender g, GrammaticalNumber num, GrammaticalCase c)
    : Word(rawWord, PartOfSpeech::Adjective, g, num, 0, c) {
}

void Adjective::changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) {
    number = newNumber;
    wordCase = newCase;

    if (newNumber == GrammaticalNumber::Plural) {
        switch (newCase) {
        case GrammaticalCase::Nominative:   ending = "і"; break;
        case GrammaticalCase::Genitive:     ending = "их"; break;
        case GrammaticalCase::Dative:       ending = "им"; break;
        case GrammaticalCase::Accusative:   ending = "і"; break;
        case GrammaticalCase::Instrumental: ending = "ими"; break;
        case GrammaticalCase::Locative:     ending = "их"; break;
        case GrammaticalCase::Vocative:     ending = "і"; break;
        default: break;
        }
    }
    else if (gender == Gender::Feminine) {
        switch (newCase) {
        case GrammaticalCase::Nominative:   ending = "а"; break;
        case GrammaticalCase::Genitive:     ending = "ої"; break;
        case GrammaticalCase::Dative:       ending = "ій"; break;
        case GrammaticalCase::Accusative:   ending = "у"; break;
        case GrammaticalCase::Instrumental: ending = "ою"; break;
        case GrammaticalCase::Locative:     ending = "ій"; break;
        case GrammaticalCase::Vocative:     ending = "а"; break;
        default: break;
        }
    }
    else {
        switch (newCase) {
        case GrammaticalCase::Nominative:   ending = (gender == Gender::Neuter) ? "е" : "ий"; break;
        case GrammaticalCase::Genitive:     ending = "ого"; break;
        case GrammaticalCase::Dative:       ending = "ому"; break;
        case GrammaticalCase::Accusative:   ending = (gender == Gender::Neuter) ? "е" : "ого"; break;
        case GrammaticalCase::Instrumental: ending = "им"; break;
        case GrammaticalCase::Locative:     ending = "ому"; break;
        case GrammaticalCase::Vocative:     ending = (gender == Gender::Neuter) ? "е" : "ий"; break;
        default: break;
        }
    }
}

Numeral::Numeral() {
    type = PartOfSpeech::Numeral;
}

Numeral::Numeral(const std::string& rawWord, GrammaticalNumber num, GrammaticalCase c)
    : Word(rawWord, PartOfSpeech::Numeral, Gender::Masculine, num, 0, c) {
}

void Numeral::changeNumberAndCase(GrammaticalNumber newNumber, GrammaticalCase newCase) {
    number = newNumber;
    wordCase = newCase;

    switch (newCase) {
    case GrammaticalCase::Nominative:   ending = (newNumber == GrammaticalNumber::Plural) ? "і" : "ий"; break;
    case GrammaticalCase::Genitive:     ending = (newNumber == GrammaticalNumber::Plural) ? "их" : "ого"; break;
    case GrammaticalCase::Dative:       ending = (newNumber == GrammaticalNumber::Plural) ? "им" : "ому"; break;
    case GrammaticalCase::Accusative:   ending = (newNumber == GrammaticalNumber::Plural) ? "і" : "ий"; break;
    case GrammaticalCase::Instrumental: ending = (newNumber == GrammaticalNumber::Plural) ? "ими" : "им"; break;
    case GrammaticalCase::Locative:     ending = (newNumber == GrammaticalNumber::Plural) ? "их" : "ому"; break;
    case GrammaticalCase::Vocative:     ending = (newNumber == GrammaticalNumber::Plural) ? "і" : "ий"; break;
    default: break;
    }
}