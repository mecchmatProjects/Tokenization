#include "../include/BagOfWordsEncoder.h"

std::vector<int> BagOfWordsEncoder::encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const {
    size_t vocabSize = vocab.size();
    std::vector<int> bow(vocabSize, 0);
    if (vocabSize == 0) {
        return bow;
    }

    for (const auto& token : tokens) {
        int id = vocab.getTokenId(token.text);
        if (id >= 0 && static_cast<size_t>(id) < vocabSize) {
            bow[static_cast<size_t>(id)]++;
        }
    }

    return bow;
}

void BagOfWordsEncoder::encodeDocument(Document& doc, const Vocabulary& vocab) const {
    for (auto& token : doc.tokens) {
        token.tokenId = vocab.getTokenId(token.text);
    }
    doc.bowVector = encodeTokens(doc.tokens, vocab);
}

std::vector<std::pair<int, int>> BagOfWordsEncoder::getNonZeroEntries(const std::vector<int>& bowVector) {
    std::vector<std::pair<int, int>> entries;
    for (size_t id = 0; id < bowVector.size(); ++id) {
        if (bowVector[id] > 0) {
            entries.emplace_back(static_cast<int>(id), bowVector[id]);
        }
    }
    return entries;
}

int BagOfWordsEncoder::getTotalTokenCount(const std::vector<int>& bowVector) {
    int total = 0;
    for (int count : bowVector) {
        total += count;
    }
    return total;
}

std::string BagOfWordsEncoder::getName() const {
    return "BagOfWordsEncoder";
}