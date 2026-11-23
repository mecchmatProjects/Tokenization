#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include "Token.h"
#include "Dataset.h"
#include "Vocabulary.h"

struct TokenFrequencyInfo {
    std::string text;
    int tokenId = -1;
    TokenType type = TokenType::Unknown;
    size_t count = 0;
    double relativeFrequency = 0.0;
    double percentage = 0.0;
    size_t documentFrequency = 0;
};

struct TokenTypeStats {
    TokenType type = TokenType::Unknown;
    std::string typeName;
    size_t totalCount = 0;
    size_t uniqueCount = 0;
    double percentage = 0.0;
    double avgLength = 0.0;
};

struct CorpusStatistics {
    size_t documentCount = 0;
    size_t totalRawCharacters = 0;
    size_t totalTokenCharacters = 0;
    size_t totalTokens = 0;
    size_t uniqueTokens = 0;

    size_t inVocabUniqueTokens = 0;
    size_t unknownTokenOccurrences = 0;
    size_t uniqueUnknownTokens = 0;
    double unknownTokenRatio = 0.0;

    double avgDocumentLengthChars = 0.0;
    double avgDocumentLengthTokens = 0.0;
    double avgTokenLength = 0.0;
    double avgUniqueTokenLength = 0.0;
    double avgUnknownTokenLength = 0.0;

    std::unordered_map<TokenType, size_t> tokenTypeCounts;
};

class StatisticsAnalyzer {
public:
    StatisticsAnalyzer() = default;

    // Comprehensive corpus analysis
    static CorpusStatistics analyze(const Dataset& dataset);
    static CorpusStatistics analyze(const Dataset& dataset, const Vocabulary& vocab);

    // Individual metric helpers
    static size_t countDocuments(const Dataset& dataset);
    static size_t countRawCharacters(const Dataset& dataset);
    static size_t countTokenCharacters(const Dataset& dataset);
    static size_t countTotalTokens(const Dataset& dataset);
    static size_t countUniqueTokens(const Dataset& dataset);
    static size_t countUnknownTokenOccurrences(const Dataset& dataset, const Vocabulary& vocab);
    static size_t countUniqueUnknownTokens(const Dataset& dataset, const Vocabulary& vocab);
    static double calculateAverageTokenLength(const Dataset& dataset);

    // Token frequency analysis
    static std::unordered_map<std::string, size_t> countTokenFrequencies(const Dataset& dataset);
    static std::unordered_map<std::string, size_t> countDocumentFrequencies(const Dataset& dataset);
    static std::vector<TokenFrequencyInfo> getTokenFrequencyTable(
        const Dataset& dataset,
        const Vocabulary* vocab = nullptr,
        size_t topN = 0);

    // Token type distribution analysis (WORD, NUMBER, PUNCTUATION, SPECIAL, etc.)
    static std::unordered_map<TokenType, size_t> countTokensByType(const Dataset& dataset);
    static std::vector<TokenTypeStats> getTokenTypeDistribution(const Dataset& dataset);

    // Console reporting helpers
    static void printReport(const CorpusStatistics& stats,
                            const std::string& title = "Corpus Statistics");
    static void printTokenFrequencyTable(const std::vector<TokenFrequencyInfo>& table,
                                         const std::string& title = "Token Frequency Table");
    static void printTokenTypeDistribution(const std::vector<TokenTypeStats>& distribution,
                                           const std::string& title = "Token Type Distribution");
};