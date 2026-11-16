#include "sentence.h"

// Default constructor implementation
Sentence::Sentence() {
    // We don't need to do anything manually here
    // std::vector initializes itself as an empty list automatically
}

// Method to add a token to the end of the sentence
void Sentence::addToken(const Token& token) {
    tokens.push_back(token);
}

// Getter returning the read-only reference to the list of tokens
const std::vector<Token>& Sentence::getTokens() const {
    return tokens;
}

// Getter returning the size of the vector
int Sentence::getTokenCount() const {
    return tokens.size();
}