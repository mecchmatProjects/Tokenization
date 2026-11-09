#pragma once
#include <vector>
#include <string>
#include <utility>
#include "Encoder.h"

class BagOfWordsEncoder : public Encoder {
public:
    BagOfWordsEncoder() = default;

    // Builds a Bag-of-Words frequency vector of size |V| from a list of tokens
    std::vector<int> encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const;

    // Populates doc.bowVector and updates token.tokenId for a single document
    void encodeDocument(Document& doc, const Vocabulary& vocab) const override;

    // Returns non-zero (tokenId, count) pairs for compact inspection
    static std::vector<std::pair<int, int>> getNonZeroEntries(const std::vector<int>& bowVector);

    // Returns the sum of all token frequencies in the BoW vector
    static int getTotalTokenCount(const std::vector<int>& bowVector);

    std::string getName() const override;
};