#ifndef SENTENCE_H
#define SENTENCE_H

#include <vector>
#include <string>
#include "token.h" // Include Token class to store tokens

class Sentence {
private:
    int number;
    std:: string text;
    // Container for Token objects
    std::vector<Token> tokens;
    int wordCount;
    int punctuationCount;

public:
    // Constructor to initialize basic info    
    Sentence(int setNumber, const std::string& sentText);

    // Method to add a new token to the sentence
    void addToken(const Token& token);

    int getNumber() const;
    std::string getText() const;

    // Returns a read-only reference to the tokens list
    const std::vector<Token>& getTokens() const;

    // Counters    
    int getTokenCount() const;
    int getWordCount() const;
    int getPunctuationCount() const;
};

#endif