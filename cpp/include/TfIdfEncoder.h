#pragma once
#include <vector>
#include <string>
#include <utility>
#include <cstddef>
#include "Encoder.h"

class TfIdfEncoder : public Encoder {
private:
    bool smoothIdf = false; // false: IDF(t) = ln(N / df(t)), true: IDF(t) = ln((N + 1) / (df(t) + 1)) + 1
    mutable size_t totalDocuments = 0;
    mutable std::vector<int> documentFrequencies;
    mutable std::vector<double> idfVector;

public:
    explicit TfIdfEncoder(bool useSmoothIdf = false);

    // Configures whether smoothed IDF formula is used
    void setSmoothIdf(bool useSmoothIdf);
    bool isSmoothIdf() const;

    // Computes Term Frequency TF(t, d) = f(t, d) / |d| for a single document's tokens
    static std::vector<double> computeTermFrequency(const std::vector<Token>& tokens, const Vocabulary& vocab);

    // Computes Document Frequency df(t): number of documents containing each vocabulary token ID
    static std::vector<int> computeDocumentFrequency(const Dataset& dataset, const Vocabulary& vocab);

    // Computes Inverse Document Frequency IDF(t) = log(N / df(t)) across the dataset
    static std::vector<double> computeIdfVector(const Dataset& dataset, const Vocabulary& vocab, bool smooth = false);

    // Fits the encoder on the dataset by precomputing df(t) and IDF(t) for all vocabulary terms
    void fit(const Dataset& dataset, const Vocabulary& vocab) const;

    // Computes the TF-IDF vector of size |V| for a list of tokens using the fitted IDF weights
    std::vector<double> encodeTokens(const std::vector<Token>& tokens, const Vocabulary& vocab) const;

    // Populates doc.tfidfVector and updates token.tokenId for a single document
    void encodeDocument(Document& doc, const Vocabulary& vocab) const override;

    // Fits IDF weights on the dataset and encodes all documents
    void encodeDataset(Dataset& dataset, const Vocabulary& vocab) const override;

    // Accessors for precomputed corpus statistics
    size_t getTotalDocuments() const;
    const std::vector<int>& getDocumentFrequencies() const;
    const std::vector<double>& getIdfVector() const;

    // Returns non-zero (tokenId, tfidfValue) pairs for compact inspection and CSV export
    static std::vector<std::pair<int, double>> getNonZeroEntries(const std::vector<double>& tfidfVector, double epsilon = 1e-9);

    std::string getName() const override;
};