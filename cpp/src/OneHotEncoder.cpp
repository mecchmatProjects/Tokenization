#include "../include/OneHotEncoder.h"

std::vector<int> OneHotEncoder::encodeSingleToken(const Token& token, const Vocabulary& vocab) const {
    size_t vocabSize = vocab.size();
    std::vector<int> oneHot(vocabSize, 0);
    if (vocabSize == 0) {
        return oneHot;
    }

    int id = vocab.getTokenId(token.text);
    if (id >= 0 && static_cast<size_t>(id) < vocabSize) {
        oneHot[static_cast<size_t>(id)] = 1;
    }

    return oneHot;
}

std::vector<std::vector<int>> OneHotEncoder::encodeTokensMatrix(const std::vector<Token>& tokens, const Vocabulary& vocab) const {
    std::vector<std::vector<int>> matrix;
    matrix.reserve(tokens.size());

    for (const auto& token : tokens) {
        matrix.push_back(encodeSingleToken(token, vocab));
    }

    return matrix;
}

std::vector<int> OneHotEncoder::encodeTokensBinary(const std::vector<Token>& tokens, const Vocabulary& vocab) const {
    size_t vocabSize = vocab.size();
    std::vector<int> binaryVec(vocabSize, 0);
    if (vocabSize == 0) {
        return binaryVec;
    }

    for (const auto& token : tokens) {
        int id = vocab.getTokenId(token.text);
        if (id >= 0 && static_cast<size_t>(id) < vocabSize) {
            binaryVec[static_cast<size_t>(id)] = 1;
        }
    }

    return binaryVec;
}

void OneHotEncoder::encodeDocument(Document& doc, const Vocabulary& vocab) const {
    for (auto& token : doc.tokens) {
        token.tokenId = vocab.getTokenId(token.text);
    }
    doc.oneHotVector = encodeTokensBinary(doc.tokens, vocab);
}

double OneHotEncoder::calculateVectorSparsity(const std::vector<int>& vec) {
    if (vec.empty()) {
        return 0.0;
    }

    size_t zeroCount = 0;
    for (int val : vec) {
        if (val == 0) {
            ++zeroCount;
        }
    }

    return static_cast<double>(zeroCount) / static_cast<double>(vec.size());
}

double OneHotEncoder::calculateMatrixSparsity(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty() || matrix[0].empty()) {
        return 0.0;
    }

    size_t totalElements = 0;
    size_t zeroCount = 0;

    for (const auto& row : matrix) {
        totalElements += row.size();
        for (int val : row) {
            if (val == 0) {
                ++zeroCount;
            }
        }
    }

    return totalElements > 0
        ? static_cast<double>(zeroCount) / static_cast<double>(totalElements)
        : 0.0;
}

double OneHotEncoder::calculateTheoreticalTokenSparsity(size_t vocabSize) {
    if (vocabSize <= 1) {
        return 0.0;
    }
    return static_cast<double>(vocabSize - 1) / static_cast<double>(vocabSize);
}

size_t OneHotEncoder::estimateMatrixMemoryBytes(size_t totalTokens, size_t vocabSize) {
    return totalTokens * vocabSize * sizeof(int);
}

std::string OneHotEncoder::getName() const {
    return "OneHotEncoder";
}