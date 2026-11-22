#include "token.h"

// Constructor implementation: assigning incoming values to the private fields
Token::Token(const std::string& tokenText, const std::string& normText, 
             TokenType tokenType, int tokenPosition, int sentNumber) {
    text = tokenText;
    normalizedText = normText;
    type = tokenType;
    position = tokenPosition;
    sentenceNumber = sentNumber;
    lemma = ""; // The lemma is empty at the beginning
}

// Getters returning the private filds
std::string Token::getText() const {
    return text;
}

std::string Token::getNormalizedText() const {
    return normalizedText;
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

int Token::getSentenceNumber() const {
    return sentenceNumber;
}

// Setter to update the lemma later
void Token::setLemma(const std::string& tokenLemma) {
    lemma = tokenLemma;
}