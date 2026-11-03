#pragma once
#include <string>

// Token types according to section 6 of the specification
enum class TokenType {
    Word,
    Number,
    Punctuation,
    Special,
    Whitespace,
    Unknown
};

// Helper function to convert TokenType to string for console and CSV output (Days 11-12)
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Word:        return "WORD";
        case TokenType::Number:      return "NUMBER";
        case TokenType::Punctuation: return "PUNCTUATION";
        case TokenType::Special:     return "SPECIAL";
        case TokenType::Whitespace:  return "WHITESPACE";
        default:                     return "UNKNOWN";
    }
}

// Core Token structure
struct Token {
    std::string text;           // Token text (or normalized form after Role 2 module)
    TokenType type;             // Token category
    size_t position = 0;        // Token index in the document (0, 1, 2...)
    size_t sentenceNumber = 0;  // Sentence index
    int tokenId = -1;           // Numeric ID from Vocabulary (-1 means unassigned)

    Token() = default;

    Token(std::string t, TokenType tp, size_t pos = 0, size_t sent = 0, int id = -1)
        : text(std::move(t)), type(tp), position(pos), sentenceNumber(sent), tokenId(id) {}
};