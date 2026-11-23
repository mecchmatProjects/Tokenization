#include "../include/Vocabulary.h"
#include <algorithm>
#include <utility>

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

bool Vocabulary::isSpecialToken(const std::string& tokenText) const {
    return tokenText == PAD_TOKEN ||
           tokenText == UNK_TOKEN ||
           tokenText == START_TOKEN ||
           tokenText == END_TOKEN;
}

int Vocabulary::addToken(const std::string& tokenText) {
    tokenFrequencies[tokenText]++;

    auto it = tokenToId.find(tokenText);
    if (it != tokenToId.end()) {
        return it->second;
    }

    int assignedId = nextId++;
    tokenToId[tokenText] = assignedId;
    idToToken[assignedId] = tokenText;

    return assignedId;
}

void Vocabulary::buildFromDataset(const Dataset& dataset, int minFrequency, size_t maxVocabularySize) {
    clear();

    if (minFrequency < 1) {
        minFrequency = 1;
    }

    // Step 1: Count raw token frequencies across all documents in O(T)
    std::unordered_map<std::string, int> rawCounts;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            if (!isSpecialToken(token.text)) {
                rawCounts[token.text]++;
            }
        }
    }

    // Step 2: Filter tokens by minFrequency and accumulate filtered-out counts into <UNK>
    std::vector<std::pair<std::string, int>> candidates;
    int unkCount = 0;

    for (const auto& [text, count] : rawCounts) {
        if (count >= minFrequency) {
            candidates.emplace_back(text, count);
        } else {
            unkCount += count;
        }
    }

    // Step 3: Sort candidates by frequency descending, then alphabetically in O(V log V)
    std::sort(candidates.begin(), candidates.end(),
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                  if (a.second != b.second) {
                      return a.second > b.second;
                  }
                  return a.first < b.first;
              });

    // Step 4: Apply maxVocabularySize limit (accounting for the 4 reserved special tokens)
    size_t maxRegularTokens = candidates.size();
    if (maxVocabularySize > 0) {
        if (maxVocabularySize <= size()) {
            maxRegularTokens = 0;
        } else {
            size_t allowed = maxVocabularySize - size();
            if (allowed < maxRegularTokens) {
                maxRegularTokens = allowed;
            }
        }
    }

    // Step 5: Register surviving tokens and route truncated tokens to <UNK>
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (i < maxRegularTokens) {
            int assignedId = nextId++;
            const std::string& text = candidates[i].first;
            int count = candidates[i].second;

            tokenToId[text] = assignedId;
            idToToken[assignedId] = text;
            tokenFrequencies[text] = count;
        } else {
            unkCount += candidates[i].second;
        }
    }

    tokenFrequencies[UNK_TOKEN] = unkCount;
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
    return UNK_ID; // Fallback to <UNK> ID for out-of-vocabulary or filtered tokens
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