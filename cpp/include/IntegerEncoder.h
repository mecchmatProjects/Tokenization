#pragma once
#include <vector>
#include <string>
#include "Encoder.h"

class IntegerEncoder : public Encoder {
private:
    bool addBoundaryTokens = false;

public:
    explicit IntegerEncoder(bool includeStartEnd = false);

    // Configures whether <START> and <END> tokens are added around the sequence
    void setAddBoundaryTokens(bool includeStartEnd);
    bool getAddBoundaryTokens() const;

    // Converts a vector of Token objects into a sequence of numeric vocabulary IDs
    std::vector<int> encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const;

    // Reconstructs token strings from a sequence of numeric IDs
    std::vector<std::string> decodeSequence(const std::vector<int>& sequence, const Vocabulary& vocab) const;

    // Populates doc.integerSequence and updates token.tokenId for a single document
    void encodeDocument(Document& doc, const Vocabulary& vocab) const override;

    std::string getName() const override;
};