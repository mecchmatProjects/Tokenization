#include "preprocessor.h"
#include <cctype>
#include <algorithm>



// Applies all preprocessing steps in the correct logical order
std::string Preprocessor::process(const std::string& rawText, bool removePunctuation, bool toLowerCase) {
    std::string text = rawText;

    text = removeControlCharacters(text);
    text = normalizeLineBreaks(text);
    text = normalizeSpaces(text);
    text = normalizeQuotesAndDashes(text);

    if (toLowerCase) {
        text = convertToLowerCase(text);
    }

    if (removePunctuation) {
        text = stripPunctuation(text);
    }

    // Trim leading and trailing spaces if any remain
    if (!text.empty() && text.front() == ' ') text.erase(0, 1);
    if (!text.empty() && text.back() == ' ') text.pop_back();

    return text;
}


// Removes unprintable control characters (like \a, \b, \v) but keeps \n, \t and \r
std::string Preprocessor::removeControlCharacters(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (char c : text) {
        if (!std::iscntrl(static_cast<unsigned char>(c)) || c == '\n' || c == '\t' || c == '\r') {
            result += c;
        }
    }
    return result;
}


// Replaces different line break formats (\r\n or \r) with standart \n
std::string Preprocessor::normalizeLineBreaks(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == '\r') {
            result += '\n';
            // Skip the next character if it's \n to handle \r\n (Windows style)
            if (i + 1 < text.length() && text[i + 1] == '\n') {
                ++i;
            }
        } else {
            result += text[i];
        }
    }
    return result;
}


// Compresses multiple consecutive spaces and tabs into a single space
std::string Preprocessor::normalizeSpaces(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    bool inSpace = false;

    for (char c : text) {
        if (c == ' ' || c == '\t') {
            if (!inSpace) {
                result += ' ';
                inSpace = true;
            }
        } else {
            result += c;
            inSpace = false;
        }
    }
    return result;
}


// Normalizes typographical quotes and long dashes into standard ASCII equivalents
std::string Preprocessor::normalizeQuotesAndDashes(const std::string& text) {
    std::string result = text;

    // A simple find-and-replace lambda helper tp keep code DRY (Don`t Repeat Yourself)
    auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    // Nor,alize dashes
    replaceAll(result, "—", "-");
    replaceAll(result, "–", "-");

    // Normalize quotes
    replaceAll(result, "«", "\"");
    replaceAll(result, "»", "\"");
    replaceAll(result, "“", "\"");
    replaceAll(result, "”", "\"");

    return result;
}


// Converts English ASCII characters to lowercase
// Note: Ukrainian UTF-8 lowercase conversion usually requires external libraries like ICU,
// so this handles basic standard characters safely
std::string Preprocessor::convertToLowerCase(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}


// Removes all punctuation characters, keeping only letters, numbers, and spaces
std::string Preprocessor::stripPunctuation(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (char c : text) {
        // We do not strip spaces, only actual punctuation
        if (!std::ispunct(static_cast<unsigned char>(c))) {
            result += c;
        }
    }
    return result;
}