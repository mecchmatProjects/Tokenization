#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include "Encoder.h"

class OneHotEncoder : public Encoder {
public:
    OneHotEncoder() = default;

    // Generates a 1D binary One-Hot vector of size |V| for a single token
    std::vector<int> encodeSingleToken(const Token& token, const Vocabulary& vocab) const;

    // Generates a 2D One-Hot matrix (K x |V|) where each row is a token's One-Hot vector
    std::vector<std::vector<int>> encodeTokensMatrix(const std::vector<Token>& tokens, const Vocabulary& vocab) const;

    // Generates a 1D binary presence vector of size |V| for an entire token list (1 if present, 0 if absent)
    std::vector<int> encodeTokensBinary(const std::vector<Token>& tokens, const Vocabulary& vocab) const;

    // Populates doc.oneHotVector and updates token.tokenId for a single document
    void encodeDocument(Document& doc, const Vocabulary& vocab) const override;

    // Sparsity analysis utilities (fraction of zero elements in [0.0, 1.0])
    static double calculateVectorSparsity(const std::vector<int>& vec);
    static double calculateMatrixSparsity(const std::vector<std::vector<int>>& matrix);
    static double calculateTheoreticalTokenSparsity(size_t vocabSize);
    static size_t estimateMatrixMemoryBytes(size_t totalTokens, size_t vocabSize);

    std::string getName() const override;
};