#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "token.h"

class Lemmatizer {
private:
    // Hash map storing pairs of (normalized word form -> base lemma)
    std::unordered_map<std::string, std::string> dictionary;

    // Single-responsibility helper to parse one line from a dictionary file
    static bool parseDictionaryLine(const std::string& line, std::string& wordForm, std::string& lemma);

public:
    Lemmatizer();
    explicit Lemmatizer(const std::string& dictionaryPath);

    // Loads wordform-lemma pairs from a text file
    bool loadDictionary(const std::string& filePath);

    // Adds a single rule manually
    void addRule(const std::string& wordForm, const std::string& lemma);

    // Returns the base lemma for a single word
    std::string lemmatizeWord(const std::string& word) const;

    // Updates the lemma field of a single token if it is a WORD
    void lemmatizeToken(Token& token) const;

    // Lemmatizes a whole vector of tokens in place
    void lemmatizeTokens(std::vector<Token>& tokens) const;

    // Getter for the number of loaded dictionary entries
    size_t getDictionarySize() const;
};