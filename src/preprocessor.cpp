#include "preprocessor.h"
#include <cctype>
#include <algorithm>

#include <locale>
#include <codecvt>
#include <cwctype>



// Suppress C++17 deprecation warnings for codecvt_utf8
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

// Helper function to convert standerd UTF-8 to wide string
std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}


// Helper function to convert wide string back to standard UTF-8 string
std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

#pragma clang diagnostic pop


// Replace Unicode horizontal ellipsis (...) with three standart dots (...)
std::string Preprocessor::normalizeEllipsis(const std::string& text) {
    std::string result = text;
    const std::string unicodeEllipsis = "\xE2\x80\xA6"; // UTF-8 encoding for '...'
    const std::string asciiEllipsis = "...";

    size_t pos = 0;
    while ((pos = result.find(unicodeEllipsis, pos)) != std::string::npos) {
        result.replace(pos, unicodeEllipsis.length(), asciiEllipsis);
        pos += asciiEllipsis.length();
    }
    return result;
}



// Applies all preprocessing steps in the correct logical order
std::string Preprocessor::process(const std::string& rawText, bool removePunctuation, bool toLowerCase) {
    std::string text = rawText;

    text = removeControlCharacters(text);
    text = normalizeLineBreaks(text);
    text = normalizeSpaces(text);
    text = normalizeQuotesAndDashes(text);
    text = normalizeEllipsis(text);

    if (toLowerCase) {
        text = convertToLowerCase(text);
    }

    if (removePunctuation) {
        text = stripPunctuation(text);
    }

    // Trim all leading and trailing spaces if any remain
    while (!text.empty() && text.front() == ' ') {
        text.erase(0, 1);
    }
    while (!text.empty() && text.back() == ' ') {
        text.pop_back();
    }

    return text;
}


// Removes unprintable control characters (like \a, \b, \v) but keeps \n, \t and \r
std::string Preprocessor::removeControlCharacters(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (char c : text) {
        unsigned char uc = static_cast<unsigned char>(c);
        // Keep printable ASCII characters (>= 32, except 127 DEL)
        // and all bytes > 127 (required for UTF-8 multibyte sequences).
        // Also explicitly allow standard formatting characters: \n, \t, \r
        if ((uc >= 32 && uc != 127) || c == '\n' || c == '\t' || c == '\r') {
            result += c;
        }
    }
    return result;
}


// Replaces different line break formats (\r\n or \r) with standard \n
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

    // A simple find-and-replace lambda helper to keep code DRY (Don`t Repeat Yourself)
    auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    // Normalize dashes
    replaceAll(result, "—", "-");
    replaceAll(result, "–", "-");

    // Normalize quotes
    replaceAll(result, "«", "\"");
    replaceAll(result, "»", "\"");
    replaceAll(result, "“", "\"");
    replaceAll(result, "”", "\"");

    return result;
}


// Converts UTF-8 text (including Ukrainian Cyrillic and Latin) to lowercase
// using wide characters and the active system locale
std::string Preprocessor::convertToLowerCase(const std::string& text) {
    // Convert to wide string for safe multibyte character handling
    std::wstring wtext = utf8_to_wstring(text);
    
    for (wchar_t& wc : wtext) {
        // Apply wide-character lowercase conversion
        wc = std::towlower(wc);
    }
    
    // Convert back to UTF-8
    return wstring_to_utf8(wtext);
}


// Removes all punctuation characters, keeping only letters, numbers, and spaces
std::string Preprocessor::stripPunctuation(const std::string& text) {
    std::wstring wtext = utf8_to_wstring(text);
    std::wstring result;
    result.reserve(wtext.size());

    for (wchar_t wc : wtext) {
        // Keep the character if it is NOT a punctuation mark
        if (!std::iswpunct(wc)) {
            result += wc;
        }
    }
    
    return wstring_to_utf8(result);
}