#pragma once

#include <string>

class Preprocessor {
public:
    // Main function that applies all preprocessing steps in sequence
    // It takes the raw text and returns a cleaned^ normalized copy
    static std::string process(const std::string& rawText, bool removePunctuation = false, bool toLowerCase = false);

    // Public helper to convert UTF-8 text to lowercase
    static std::string convertToLowerCase(const std::string& text);

private:
    // Single-responsibility helper functions for isolated text transformations
    static std::string removeControlCharacters(const std::string& text);
    static std::string normalizeLineBreaks(const std::string& text);
    static std::string normalizeSpaces(const std::string& text);
    static std::string normalizeQuotesAndDashes(const std::string& text);
    static std::string normalizeEllipsis(const std::string& text);
    static std::string stripPunctuation(const std::string& text);
};