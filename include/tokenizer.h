#pragma once

#include <string>
#include <vector>
#include "token.h"

class Tokenizer {
public:
    // Main function that preprocessed text and splits it into tokens
    static std::vector<Token> tokenize(const std::string& text);

private:
    //Single-responsibility helper functions for token analysis
    static bool isPunctuation(char c);
    static bool isSentenceEnder(char c);
    static bool isNumber(const std::string& str);

    // Helper to determine the type of an accumulated token string
    static TokenType determineTokenType(const std::string& tokenString);
};