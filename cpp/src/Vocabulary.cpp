#include "../include/Vocabulary.h"

Vocabulary::Vocabulary() {
    initSpecialTokens();
}

void Vocabulary::initSpecialTokens() {
    const std::vector<std::string> specialTokens = {
        PAD_TOKEN,
        UNK_TOKEN,
        START_TOKEN,
        END_TOKEN
    };

    for (const auto& token : specialTokens) {
        int assignedId = nextId++;
        tokenToId[token] = assignedId;
        idToToken[assignedId] = token;
        tokenFrequencies[token] = 0;
    }
}

int Vocabulary::addToken(const std::string& tokenText) {
    // Increment token frequency count in the corpus
    tokenFrequencies[tokenText]++;

    // Check if the token already has an assigned ID
    auto it = tokenToId.find(tokenText);
    if (it != tokenToId.end()) {
        return it->second;
    }

    // Assign a new unique ID for an unseen token
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
    return UNK_ID; // Fallback to <UNK> ID for out-of-vocabulary tokens
}

std::string Vocabulary::getTokenText(int id) const {
    auto it = idToToken.find(id);
    if (it != idToToken.end()) {
        return it->second;
    }
    return "";
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
    initSpecialTokens();
}

const std::unordered_map<std::string, int>& Vocabulary::getTokenToIdMap() const {
    return tokenToId;
}