#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Dataset.h"

class Vocabulary {
public:
    // Fixed IDs for standard special tokens
    static constexpr int PAD_ID = 0;
    static constexpr int UNK_ID = 1;
    static constexpr int START_ID = 2;
    static constexpr int END_ID = 3;

    // String representations of special tokens
    inline static const std::string PAD_TOKEN = "<PAD>";
    inline static const std::string UNK_TOKEN = "<UNK>";
    inline static const std::string START_TOKEN = "<START>";
    inline static const std::string END_TOKEN = "<END>";

private:
    std::unordered_map<std::string, int> tokenToId;
    std::unordered_map<int, std::string> idToToken;
    std::unordered_map<std::string, int> tokenFrequencies;
    int nextId = 0;

    // Registers special tokens at fixed indices (0..3) with initial frequency 0
    void initSpecialTokens();

    // Checks if a token is one of the reserved special tokens
    bool isSpecialToken(const std::string& tokenText) const;

public:
    Vocabulary();

    // Adds a single token to the vocabulary and increments its frequency
    int addToken(const std::string& tokenText);

    // Builds sorted and filtered vocabulary from the dataset
    // minFrequency: minimum occurrences required to keep a token
    // maxVocabularySize: maximum total tokens including special tokens (0 = unlimited)
    void buildFromDataset(const Dataset& dataset, int minFrequency = 1, size_t maxVocabularySize = 0);

    // Assigns vocabulary IDs directly to Token objects inside the dataset
    void assignIdsToDataset(Dataset& dataset) const;

    // Forward lookup: Token string -> Numeric ID (returns UNK_ID if not found)
    int getTokenId(const std::string& tokenText) const;

    // Reverse lookup: Numeric ID -> Token string (returns empty string if not found)
    std::string getTokenText(int id) const;

    // Returns how many times a token appeared in the corpus
    int getTokenFrequency(const std::string& tokenText) const;

    // Checks if a token exists in the vocabulary
    bool contains(const std::string& tokenText) const;

    // Returns total number of tokens in the vocabulary (including special tokens)
    size_t size() const;

    // Clears all internal maps and re-initializes special tokens
    void clear();

    // Read-only access to the token->ID map for iteration and export
    const std::unordered_map<std::string, int>& getTokenToIdMap() const;
};