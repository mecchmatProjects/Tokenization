#include "../include/IntegerEncoder.h"

IntegerEncoder::IntegerEncoder(bool includeStartEnd, size_t maxSeqLength)
    : addBoundaryTokens(includeStartEnd), maxLength(maxSeqLength) {}

void IntegerEncoder::setAddBoundaryTokens(bool includeStartEnd) {
    addBoundaryTokens = includeStartEnd;
}

bool IntegerEncoder::getAddBoundaryTokens() const {
    return addBoundaryTokens;
}

void IntegerEncoder::setMaxLength(size_t maxSeqLength) {
    maxLength = maxSeqLength;
}

size_t IntegerEncoder::getMaxLength() const {
    return maxLength;
}

void IntegerEncoder::setPostPadding(bool padAtEnd) {
    postPadding = padAtEnd;
}

bool IntegerEncoder::isPostPadding() const {
    return postPadding;
}

void IntegerEncoder::setPostTruncation(bool truncateFromEnd) {
    postTruncation = truncateFromEnd;
}

bool IntegerEncoder::isPostTruncation() const {
    return postTruncation;
}

std::vector<int> IntegerEncoder::alignSequence(const std::vector<int>& sequence) const {
    if (maxLength == 0 || sequence.size() == maxLength) {
        return sequence;
    }

    // Truncation when sequence exceeds maxLength
    if (sequence.size() > maxLength) {
        if (postTruncation) {
            return std::vector<int>(sequence.begin(), sequence.begin() + static_cast<std::ptrdiff_t>(maxLength));
        } else {
            return std::vector<int>(sequence.end() - static_cast<std::ptrdiff_t>(maxLength), sequence.end());
        }
    }

    // Padding with Vocabulary::PAD_ID (0) when sequence is shorter than maxLength
    size_t padCount = maxLength - sequence.size();
    std::vector<int> aligned;
    aligned.reserve(maxLength);

    if (postPadding) {
        aligned.insert(aligned.end(), sequence.begin(), sequence.end());
        aligned.insert(aligned.end(), padCount, Vocabulary::PAD_ID);
    } else {
        aligned.insert(aligned.end(), padCount, Vocabulary::PAD_ID);
        aligned.insert(aligned.end(), sequence.begin(), sequence.end());
    }

    return aligned;
}

std::vector<int> IntegerEncoder::encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const {
    std::vector<int> sequence;
    size_t reserveSize = tokens.size() + (addBoundaryTokens ? 2 : 0);
    sequence.reserve(reserveSize);

    if (addBoundaryTokens) {
        sequence.push_back(Vocabulary::START_ID);
    }

    for (const auto& token : tokens) {
        sequence.push_back(vocab.getTokenId(token.text));
    }

    if (addBoundaryTokens) {
        sequence.push_back(Vocabulary::END_ID);
    }

    if (maxLength > 0) {
        return alignSequence(sequence);
    }

    return sequence;
}

std::vector<std::string> IntegerEncoder::decodeSequence(const std::vector<int>& sequence, const Vocabulary& vocab) const {
    std::vector<std::string> decoded;
    decoded.reserve(sequence.size());

    for (int id : sequence) {
        std::string text = vocab.getTokenText(id);
        if (text.empty()) {
            decoded.push_back(Vocabulary::UNK_TOKEN);
        } else {
            decoded.push_back(text);
        }
    }

    return decoded;
}

void IntegerEncoder::encodeDocument(Document& doc, const Vocabulary& vocab) const {
    for (auto& token : doc.tokens) {
        token.tokenId = vocab.getTokenId(token.text);
    }
    doc.integerSequence = encodeTokens(doc.tokens, vocab);
}

size_t IntegerEncoder::countPaddingTokens(const std::vector<int>& sequence) {
    size_t count = 0;
    for (int id : sequence) {
        if (id == Vocabulary::PAD_ID) {
            ++count;
        }
    }
    return count;
}

size_t IntegerEncoder::countDatasetPaddingTokens(const Dataset& dataset) {
    size_t totalPad = 0;
    for (const auto& doc : dataset.getDocuments()) {
        totalPad += countPaddingTokens(doc.integerSequence);
    }
    return totalPad;
}

double IntegerEncoder::calculateDatasetPaddingRatio(const Dataset& dataset) {
    size_t totalTokens = 0;
    size_t totalPad = 0;

    for (const auto& doc : dataset.getDocuments()) {
        totalTokens += doc.integerSequence.size();
        totalPad += countPaddingTokens(doc.integerSequence);
    }

    if (totalTokens == 0) {
        return 0.0;
    }

    return static_cast<double>(totalPad) / static_cast<double>(totalTokens);
}

std::string IntegerEncoder::getName() const {
    return "IntegerEncoder";
}