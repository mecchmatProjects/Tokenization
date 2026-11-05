#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Dataset.h"

class Vocabulary {
private:
    std::unordered_map<std::string, int> tokenToId;
    std::unordered_map<int, std::string> idToToken;
    std::unordered_map<std::string, int> tokenFrequencies;
    int nextId = 0;

public:
    Vocabulary() = default;

    // Adds a single token to the vocabulary and updates its frequency
    int addToken(const std::string& tokenText);

    // Builds the vocabulary from all documents in the dataset
    void buildFromDataset(const Dataset& dataset);

    // Assigns vocabulary IDs directly to Token objects inside the dataset
    void assignIdsToDataset(Dataset& dataset) const;

    // Forward lookup: Token string -> Numeric ID (returns -1 if not found)
    int getTokenId(const std::string& tokenText) const;

    // Reverse lookup: Numeric ID -> Token string (returns empty string if not found)
    std::string getTokenText(int id) const;

    // Returns how many times a token appeared in the corpus
    int getTokenFrequency(const std::string& tokenText) const;

    // Checks if a token exists in the vocabulary
    bool contains(const std::string& tokenText) const;

    // Returns total number of unique tokens in the vocabulary
    size_t size() const;

    // Clears all internal maps and resets ID counter
    void clear();

    // Read-only access to the token->ID map for iteration and console/CSV output
    const std::unordered_map<std::string, int>& getTokenToIdMap() const;
};