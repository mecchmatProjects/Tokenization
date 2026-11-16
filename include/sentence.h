#ifndef SENTENCE_H
#define SENTENCE_H

#include <vector>
#include "token.h" // Include Token class to store tokens

class Sentence {
private:
    // Container for Token objects
    std::vector<Token> tokens;

public:
    // Constructor
    Sentence();

    // Method to add a new token to the sentence
    void addToken(const Token& token);

    // Returns a read-only reference to the tokens list
    const std::vector<Token>& getTokens() const;

    // Quick getter for the number of tokens
    int getTokenCount() const;
};

#endif