#include "../include/TfIdfEncoder.h"
#include <cmath>
#include <unordered_set>

TfIdfEncoder::TfIdfEncoder(bool useSmoothIdf)
    : smoothIdf(useSmoothIdf) {}

void TfIdfEncoder::setSmoothIdf(bool useSmoothIdf) {
    smoothIdf = useSmoothIdf;
}

bool TfIdfEncoder::isSmoothIdf() const {
    return smoothIdf;
}

std::vector<double> TfIdfEncoder::computeTermFrequency(const std::vector<Token>& tokens, const Vocabulary& vocab) {
    size_t vocabSize = vocab.size();
    std::vector<double> tf(vocabSize, 0.0);
    if (vocabSize == 0 || tokens.empty()) {
        return tf;
    }

    for (const auto& token : tokens) {
        int id = vocab.getTokenId(token.text);
        if (id >= 0 && static_cast<size_t>(id) < vocabSize) {
            tf[static_cast<size_t>(id)] += 1.0;
        }
    }

    double totalTokens = static_cast<double>(tokens.size());
    for (double& val : tf) {
        val /= totalTokens;
    }

    return tf;
}

std::vector<int> TfIdfEncoder::computeDocumentFrequency(const Dataset& dataset, const Vocabulary& vocab) {
    size_t vocabSize = vocab.size();
    std::vector<int> df(vocabSize, 0);
    if (vocabSize == 0) {
        return df;
    }

    for (const auto& doc : dataset.getDocuments()) {
        std::unordered_set<int> uniqueIdsInDoc;
        for (const auto& token : doc.tokens) {
            int id = vocab.getTokenId(token.text);
            if (id >= 0 && static_cast<size_t>(id) < vocabSize) {
                uniqueIdsInDoc.insert(id);
            }
        }
        for (int id : uniqueIdsInDoc) {
            df[static_cast<size_t>(id)]++;
        }
    }

    return df;
}

std::vector<double> TfIdfEncoder::computeIdfVector(const Dataset& dataset, const Vocabulary& vocab, bool smooth) {
    size_t vocabSize = vocab.size();
    std::vector<double> idf(vocabSize, 0.0);
    size_t numDocs = dataset.size();
    if (vocabSize == 0 || numDocs == 0) {
        return idf;
    }

    std::vector<int> df = computeDocumentFrequency(dataset, vocab);
    double n = static_cast<double>(numDocs);

    for (size_t id = 0; id < vocabSize; ++id) {
        if (df[id] > 0) {
            if (smooth) {
                idf[id] = std::log((n + 1.0) / (static_cast<double>(df[id]) + 1.0)) + 1.0;
            } else {
                idf[id] = std::log(n / static_cast<double>(df[id]));
            }
        }
    }

    return idf;
}

void TfIdfEncoder::fit(const Dataset& dataset, const Vocabulary& vocab) const {
    totalDocuments = dataset.size();
    documentFrequencies = computeDocumentFrequency(dataset, vocab);
    idfVector = computeIdfVector(dataset, vocab, smoothIdf);
}

std::vector<double> TfIdfEncoder::encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const {
    size_t vocabSize = vocab.size();
    std::vector<double> tfidf(vocabSize, 0.0);
    if (vocabSize == 0 || tokens.empty()) {
        return tfidf;
    }

    std::vector<double> tf = computeTermFrequency(tokens, vocab);
    for (size_t id = 0; id < vocabSize; ++id) {
        double idf = (id < idfVector.size()) ? idfVector[id] : 0.0;
        tfidf[id] = tf[id] * idf;
    }

    return tfidf;
}

void TfIdfEncoder::encodeDocument(Document& doc, const Vocabulary& vocab) const {
    for (auto& token : doc.tokens) {
        token.tokenId = vocab.getTokenId(token.text);
    }
    doc.tfidfVector = encodeTokens(doc.tokens, vocab);
}

void TfIdfEncoder::encodeDataset(Dataset& dataset, const Vocabulary& vocab) const {
    fit(dataset, vocab);
    for (auto& doc : dataset.getDocuments()) {
        encodeDocument(doc, vocab);
    }
}

size_t TfIdfEncoder::getTotalDocuments() const {
    return totalDocuments;
}

const std::vector<int>& TfIdfEncoder::getDocumentFrequencies() const {
    return documentFrequencies;
}

const std::vector<double>& TfIdfEncoder::getIdfVector() const {
    return idfVector;
}

std::vector<std::pair<int, double>> TfIdfEncoder::getNonZeroEntries(const std::vector<double>& tfidfVector, double epsilon) {
    std::vector<std::pair<int, double>> entries;
    for (size_t id = 0; id < tfidfVector.size(); ++id) {
        if (std::abs(tfidfVector[id]) > epsilon) {
            entries.emplace_back(static_cast<int>(id), tfidfVector[id]);
        }
    }
    return entries;
}

std::string TfIdfEncoder::getName() const {
    return "TfIdfEncoder";
}