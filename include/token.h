#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Enumeration for token types
enum class TokenType {
    WORD,
    NUMBER,
    PUNCTUATION,
    SPECIAL
};

class Token {
private:
    std::string text; // original text of the token
    std::string lemma; // normalized form (initially empty)
    TokenType type; // type from the enum above
    int position; // sequential position in the text

public:
    // Constructor to initialize the token
    Token(const std::string& tokenText, TokenType tokenType, int tokenPosition);

    // Getters
    std::string getText() const;
    std::string getLemma() const;
    TokenType getType() const;
    int getPosition() const;

    // Setter for the lemma (assigned later during lemmatization)
    void setLemma(const std::string& tokenLemma);
};

#endif