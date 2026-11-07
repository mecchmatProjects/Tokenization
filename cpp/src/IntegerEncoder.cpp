#include "../include/IntegerEncoder.h"

IntegerEncoder::IntegerEncoder(bool includeStartEnd)
    : addBoundaryTokens(includeStartEnd) {}

void IntegerEncoder::setAddBoundaryTokens(bool includeStartEnd) {
    addBoundaryTokens = includeStartEnd;
}

bool IntegerEncoder::getAddBoundaryTokens() const {
    return addBoundaryTokens;
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

std::string IntegerEncoder::getName() const {
    return "IntegerEncoder";
}