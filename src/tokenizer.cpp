#include "tokenizer.h"
#include "preprocessor.h"
#include <cctype>



// Checks if a character is a standard punctuation mark
bool Tokenizer::isPunctuation(char c) {
    return c == '.' || c == ',' || c == '!' || c == '?' ||
           c == ':' || c == ';' || c == '-' || c == '"' ||
           c == '(' || c == ')';
}


// Checks if a character marks the end of a sentence
bool Tokenizer::isSentenceEnder(char c) {
    return c == '.' || c == '!' || c == '?';
}


// Checks if a string represents an integer or a decimal number (e.g., "42", "3.14", "12,5")
bool Tokenizer::isNumber(const std::string& str) {
    if (str.empty()) return false;
    bool hasDigit = false;
    bool hasSeparator = false;

    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (std::isdigit(uc)) {
            hasDigit = true;
        } else if ((str[i] == '.' || str[i] == ',') && !hasSeparator && i > 0 && i + 1 < str.length()) {
            hasSeparator = true;
        } else {
            return false;
        }
    }
    return hasDigit;
}


// Assings the correct TokenType enum based on the string content
TokenType Tokenizer::determineTokenType(const std::string& tokenString) {
    if (tokenString.length() == 1 && isPunctuation(tokenString[0])) {
        return TokenType::PUNCTUATION;
    }
    if (isNumber(tokenString)) {
        return TokenType::NUMBER;
    }

    // Check if the token contains at least one letter (ASCII or UTF-8 multibyte character)
    bool hasLetter = false;
    for (char c : tokenString) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalpha(uc) || uc >= 128) {
            hasLetter = true;
            break;
        }
    }

    if (!hasLetter) {
        return TokenType::SPECIAL;
    }

    return TokenType::WORD;
}


// Main function that splits text into a vector of tokens
std::vector<Token> Tokenizer::tokenize(const std::string& text) {
    std::vector<Token> tokens;
    std::string currentTokenStr = "";

    int currentPosition = 1;
    int currentSentence = 1;
    bool sentenceEnded = false;
    bool inQuotes = false; // if in ""

    // Helper lambda to flush the accumulated word/number token
    auto flushCurrentToken = [&]() {
        if (!currentTokenStr.empty()) {
            if (sentenceEnded) {
                currentSentence++;
                sentenceEnded = false;
            }
            TokenType type = determineTokenType(currentTokenStr);
            std::string norm = Preprocessor::convertToLowerCase(currentTokenStr);
            tokens.push_back(Token(currentTokenStr, norm, type, currentPosition++, currentSentence));
            currentTokenStr = "";
        }
    };

    // Iterate through every character in the text
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        unsigned char uc = static_cast<unsigned char>(c);

        // Step 1: Handle ASCII whitespace only (never split UTF-8 multibyte characters)
        if (uc < 128 && std::isspace(uc)) {
            flushCurrentToken();
            continue;
        }

        // Step 2: Handle ellipsis "..." as a single punctuation token
        if (c == '.' && i + 2 < text.length() && text[i + 1] == '.' && text[i + 2] == '.') {
            flushCurrentToken();
            if (sentenceEnded) {
                currentSentence++;
                sentenceEnded = false;
            }
            tokens.push_back(Token("...", "...", TokenType::PUNCTUATION, currentPosition++, currentSentence));
            sentenceEnded = true;
            i += 2; // Skip the next two dots
            continue;
        }

        // Step 3: Handle decimal separator inside a number (e.g., "3.14" or "12,5")
        if ((c == '.' || c == ',') && !currentTokenStr.empty() && isNumber(currentTokenStr) &&
            i + 1 < text.length() && std::isdigit(static_cast<unsigned char>(text[i + 1]))) {
            currentTokenStr += c;
            continue;
        }

        // Step 4: Handle hyphen inside a word (e.g., "будь-який")
        if (c == '-' && !currentTokenStr.empty() && i + 1 < text.length()) {
            unsigned char nextUc = static_cast<unsigned char>(text[i + 1]);
            if (!(nextUc < 128 && std::isspace(nextUc)) && !isPunctuation(text[i + 1])) {
                currentTokenStr += c;
                continue;
            }
        }

        // Step 5: Handle standard punctuation
        if (isPunctuation(c)) {
            flushCurrentToken();

            // Keep closing parenthesis and closing quote in the sentence that just ended
            bool isClosingMark = (c == ')') || (c == '"' && inQuotes);

            // If a new sentence starts with non-ending punctuation (like opening quote or dash)
            if (sentenceEnded && !isSentenceEnder(c) && !isClosingMark) {
                currentSentence++;
                sentenceEnded = false;
            }

            // Toggle quote state after checking whether it was opening or closing
            if (c == '"') {
                inQuotes = !inQuotes;
            }

            std::string punctStr(1, c);
            tokens.push_back(Token(punctStr, punctStr, TokenType::PUNCTUATION, currentPosition++, currentSentence));

            if (isSentenceEnder(c)) {
                sentenceEnded = true;
            }
            continue;
        }
        
        // Step 6: Accumulate normal letters, digits, or special symbols
        currentTokenStr += c;
    }

    // Save the last token if the text didn't end with punctuation or space
    flushCurrentToken();
    
    return tokens;
}