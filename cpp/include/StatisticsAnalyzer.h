#pragma once
#include <cstddef>
#include <string>
#include "Dataset.h"
#include "Vocabulary.h"

// Aggregated statistical metrics for a tokenized text corpus
struct CorpusStatistics {
    size_t documentCount = 0;               // Total number of documents in the dataset
    size_t totalRawCharacters = 0;          // Total characters in raw document texts (including spaces)
    size_t totalTokenCharacters = 0;        // Total characters across all extracted tokens
    size_t totalTokens = 0;                 // Total number of token occurrences (T)
    size_t uniqueTokens = 0;                // Number of distinct surface token strings in the corpus
    size_t inVocabularyUniqueTokens = 0;    // Number of distinct corpus tokens present in Vocabulary
    size_t unknownTokens = 0;               // Total occurrences of unknown (<UNK>) tokens
    size_t uniqueUnknownTokens = 0;         // Number of distinct surface tokens mapped to <UNK>

    double averageDocumentLengthChars = 0.0;   // Average raw characters per document
    double averageDocumentLengthTokens = 0.0;  // Average tokens per document
    double averageTokenLength = 0.0;           // Mean character length across all token occurrences
    double averageUniqueTokenLength = 0.0;     // Mean character length across unique tokens
    double averageUnknownTokenLength = 0.0;    // Mean character length across unknown token occurrences
    double unknownTokenRatio = 0.0;            // Percentage/fraction of unknown tokens in [0.0, 1.0]
};

class StatisticsAnalyzer {
public:
    StatisticsAnalyzer() = default;

    // Counts total number of documents in the dataset
    static size_t countDocuments(const Dataset& dataset);

    // Counts total characters in raw text across all documents
    static size_t countTotalRawCharacters(const Dataset& dataset);

    // Counts total characters inside all extracted tokens across the dataset
    static size_t countTotalTokenCharacters(const Dataset& dataset);

    // Counts total number of extracted tokens across all documents
    static size_t countTotalTokens(const Dataset& dataset);

    // Counts distinct token strings appearing in the dataset
    static size_t countUniqueTokens(const Dataset& dataset);

    // Counts total occurrences of tokens that resolve to <UNK> or TokenType::Unknown
    static size_t countUnknownTokens(const Dataset& dataset, const Vocabulary* vocab = nullptr);

    // Counts distinct surface token strings that resolve to <UNK> or TokenType::Unknown
    static size_t countUniqueUnknownTokens(const Dataset& dataset, const Vocabulary* vocab = nullptr);

    // Calculates mean character length of all token occurrences in the dataset
    static double calculateAverageTokenLength(const Dataset& dataset);

    // Calculates mean character length of unique token strings in the dataset
    static double calculateAverageUniqueTokenLength(const Dataset& dataset);

    // Calculates mean character length of unknown (<UNK>) token occurrences
    static double calculateAverageUnknownTokenLength(const Dataset& dataset, const Vocabulary* vocab = nullptr);

    // Computes the complete statistical summary for a dataset (optionally evaluated against a Vocabulary)
    static CorpusStatistics analyze(const Dataset& dataset, const Vocabulary* vocab = nullptr);

    // Helper method to check if a token is considered unknown
    static bool isTokenUnknown(const Token& token, const Vocabulary* vocab = nullptr);
};