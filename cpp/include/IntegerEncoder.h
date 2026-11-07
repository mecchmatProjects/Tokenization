#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include "Encoder.h"

class IntegerEncoder : public Encoder {
private:
    bool addBoundaryTokens = false;
    size_t maxLength = 0;       // 0 means dynamic length (no padding or truncation)
    bool postPadding = true;    // true: pad at the end, false: pad at the beginning
    bool postTruncation = true; // true: truncate from the end, false: truncate from the beginning

public:
    explicit IntegerEncoder(bool includeStartEnd = false, size_t maxSeqLength = 0);

    // Configures whether <START> and <END> tokens are added around the sequence
    void setAddBoundaryTokens(bool includeStartEnd);
    bool getAddBoundaryTokens() const;

    // Configures fixed sequence length for padding and truncation (0 = disabled)
    void setMaxLength(size_t maxSeqLength);
    size_t getMaxLength() const;

    // Configures padding and truncation direction
    void setPostPadding(bool padAtEnd);
    bool isPostPadding() const;

    void setPostTruncation(bool truncateFromEnd);
    bool isPostTruncation() const;

    // Pads with Vocabulary::PAD_ID (0) or truncates a sequence to match maxLength
    std::vector<int> alignSequence(const std::vector<int>& sequence) const;

    // Converts a vector of Token objects into a sequence of numeric vocabulary IDs
    std::vector<int> encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const;

    // Reconstructs token strings from a sequence of numeric IDs
    std::vector<std::string> decodeSequence(const std::vector<int>& sequence, const Vocabulary& vocab) const;

    // Populates doc.integerSequence and updates token.tokenId for a single document
    void encodeDocument(Document& doc, const Vocabulary& vocab) const override;

    // Utility methods to evaluate padding memory overhead across the dataset
    static size_t countPaddingTokens(const std::vector<int>& sequence);
    static size_t countDatasetPaddingTokens(const Dataset& dataset);
    static double calculateDatasetPaddingRatio(const Dataset& dataset);

    std::string getName() const override;
};