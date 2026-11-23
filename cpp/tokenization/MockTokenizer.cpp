#include "MockTokenizer.h"

std::vector<Token> MockTokenizer::tokenize(const std::string&) const {
    return {
        Token("Hello", TokenType::Word, 0, 1),
        Token("world", TokenType::Word, 1, 1),
        Token("42", TokenType::Number, 2, 1),
        Token("!", TokenType::Punctuation, 3, 1)
    };
}
