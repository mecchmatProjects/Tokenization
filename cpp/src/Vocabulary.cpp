#include "../include/Vocabulary.h"

int Vocabulary::addToken(const std::string& tokenText) {
    // Always increment token frequency count
    tokenFrequencies[tokenText]++;

    // Check if the token already has an assigned ID
    auto it = tokenToId.find(tokenText);
    if (it != tokenToId.end()) {
        return it->second;
    }

    // Assign a new unique ID for a unseen token
    int assignedId = nextId++;
    tokenToId[tokenText] = assignedId;
    idToToken[assignedId] = tokenText;

    return assignedId;
}

void Vocabulary::buildFromDataset(const Dataset& dataset) {
    clear();

    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            addToken(token.text);
        }
    }
}

void Vocabulary::assignIdsToDataset(Dataset& dataset) const {
    for (auto& doc : dataset.getDocuments()) {
        for (auto& token : doc.tokens) {
            token.tokenId = getTokenId(token.text);
        }
    }
}

int Vocabulary::getTokenId(const std::string& tokenText) const {
    auto it = tokenToId.find(tokenText);
    if (it != tokenToId.end()) {
        return it->second;
    }
    return -1; // Token not found (will be replaced with <UNK> ID on Day 3)
}

std::string Vocabulary::getTokenText(int id) const {
    auto it = idToToken.find(id);
    if (it != idToToken.end()) {
        return it->second;
    }
    return ""; // ID not found
}

int Vocabulary::getTokenFrequency(const std::string& tokenText) const {
    auto it = tokenFrequencies.find(tokenText);
    if (it != tokenFrequencies.end()) {
        return it->second;
    }
    return 0;
}

bool Vocabulary::contains(const std::string& tokenText) const {
    return tokenToId.find(tokenText) != tokenToId.end();
}

size_t Vocabulary::size() const {
    return tokenToId.size();
}

void Vocabulary::clear() {
    tokenToId.clear();
    idToToken.clear();
    tokenFrequencies.clear();
    nextId = 0;
}

const std::unordered_map<std::string, int>& Vocabulary::getTokenToIdMap() const {
    return tokenToId;
}