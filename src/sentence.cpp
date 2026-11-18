#include "sentence.h"

// Default constructor implementation
Sentence::Sentence(int sentNumber, const std::string& sentText) {
    number = sentNumber;
    text = sentText;
    wordCount = 0;
    punctuationCount = 0;
}

// When adding a token, we automatically update our word and punctuation counters
void Sentence::addToken(const Token& token) {
    tokens.push_back(token);

    if (token.getType() == TokenType::WORD) {
        wordCount++;
    } else if (token.getType() == TokenType::PUNCTUATION) {
        punctuationCount++;
    }
}

int Sentence::getNumber() const {
    return number;
}

std::string Sentence::getText() const {
    return text;
}


// Getter returning the read-only reference to the list of tokens
const std::vector<Token>& Sentence::getTokens() const {
    return tokens;
}

// Getter returning the size of the vector
int Sentence::getTokenCount() const {
    return tokens.size();
}

int Sentence::getWordCount() const {
    return wordCount;
}

int Sentence::getPunctuationCount() const {
    return punctuationCount;
}