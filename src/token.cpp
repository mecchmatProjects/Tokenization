#include "token.h"

// Constructor implementation: assigning incoming values to the private fields
Token::Token(const std::string& tokenText, TokenType tokenType, int tokenPosition) {
    text = tokenText;
    type = tokenType;
    position = tokenPosition;
    lemma = ""; // The lemma is empty at the beginning
}

// Getters returning the private filds
std::string Token::getText() const {
    return text;
}

std::string Token::getLemma() const {
    return lemma;
}

TokenType Token::getType() const {
    return type;
}

int Token::getPosition() const {
    return position;
}

// Setter to update the lemma later
void Token::setLemma(const std::string& tokenLemma) {
    lemma = tokenLemma;
}