#pragma once

#include "TokenizerStrategy.h"

namespace tokenization_detail {

struct Character {
    char32_t value;
    std::size_t offset;
    std::size_t length;
};

std::vector<Character> decodeUtf8(const std::string& text);
TokenType classify(char32_t character);
bool endsSentence(char32_t character);

class TokenBuilder {
public:
    void append(const std::string& text, TokenType type, bool sentenceEnd);
    std::vector<Token> takeTokens();

private:
    std::vector<Token> tokens;
    std::size_t sentenceNumber = 1;
    bool pendingSentenceEnd = false;
};

}
