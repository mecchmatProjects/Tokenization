#include "../include/StatisticsAnalyzer.h"
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <iomanip>

CorpusStatistics StatisticsAnalyzer::analyze(const Dataset& dataset) {
    CorpusStatistics stats;
    stats.documentCount = dataset.size();

    std::unordered_set<std::string> uniqueSet;
    size_t uniqueCharsSum = 0;

    for (const auto& doc : dataset.getDocuments()) {
        stats.totalRawCharacters += doc.rawText.size();
        stats.totalTokens += doc.tokens.size();

        for (const auto& token : doc.tokens) {
            size_t len = token.text.size();
            stats.totalTokenCharacters += len;
            stats.tokenTypeCounts[token.type]++;

            if (uniqueSet.insert(token.text).second) {
                uniqueCharsSum += len;
            }
        }
    }

    stats.uniqueTokens = uniqueSet.size();
    stats.inVocabUniqueTokens = stats.uniqueTokens;

    if (stats.documentCount > 0) {
        stats.avgDocumentLengthChars =
            static_cast<double>(stats.totalRawCharacters) / stats.documentCount;
        stats.avgDocumentLengthTokens =
            static_cast<double>(stats.totalTokens) / stats.documentCount;
    }

    if (stats.totalTokens > 0) {
        stats.avgTokenLength =
            static_cast<double>(stats.totalTokenCharacters) / stats.totalTokens;
    }

    if (stats.uniqueTokens > 0) {
        stats.avgUniqueTokenLength =
            static_cast<double>(uniqueCharsSum) / stats.uniqueTokens;
    }

    return stats;
}

CorpusStatistics StatisticsAnalyzer::analyze(const Dataset& dataset, const Vocabulary& vocab) {
    CorpusStatistics stats = analyze(dataset);

    std::unordered_set<std::string> inVocabSet;
    std::unordered_set<std::string> unknownSet;
    size_t unknownCharsSum = 0;

    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            int id = vocab.getTokenId(token.text);
            if (id == Vocabulary::UNK_ID && token.text != Vocabulary::UNK_TOKEN) {
                stats.unknownTokenOccurrences++;
                unknownCharsSum += token.text.size();
                unknownSet.insert(token.text);
            } else {
                inVocabSet.insert(token.text);
            }
        }
    }

    stats.inVocabUniqueTokens = inVocabSet.size();
    stats.uniqueUnknownTokens = unknownSet.size();

    if (stats.totalTokens > 0) {
        stats.unknownTokenRatio =
            static_cast<double>(stats.unknownTokenOccurrences) / stats.totalTokens;
    }

    if (stats.unknownTokenOccurrences > 0) {
        stats.avgUnknownTokenLength =
            static_cast<double>(unknownCharsSum) / stats.unknownTokenOccurrences;
    }

    return stats;
}

size_t StatisticsAnalyzer::countDocuments(const Dataset& dataset) {
    return dataset.size();
}

size_t StatisticsAnalyzer::countRawCharacters(const Dataset& dataset) {
    size_t total = 0;
    for (const auto& doc : dataset.getDocuments()) {
        total += doc.rawText.size();
    }
    return total;
}

size_t StatisticsAnalyzer::countTokenCharacters(const Dataset& dataset) {
    size_t total = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            total += token.text.size();
        }
    }
    return total;
}

size_t StatisticsAnalyzer::countTotalTokens(const Dataset& dataset) {
    size_t total = 0;
    for (const auto& doc : dataset.getDocuments()) {
        total += doc.tokens.size();
    }
    return total;
}

size_t StatisticsAnalyzer::countUniqueTokens(const Dataset& dataset) {
    std::unordered_set<std::string> uniqueSet;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            uniqueSet.insert(token.text);
        }
    }
    return uniqueSet.size();
}

size_t StatisticsAnalyzer::countUnknownTokenOccurrences(const Dataset& dataset, const Vocabulary& vocab) {
    size_t count = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            int id = vocab.getTokenId(token.text);
            if (id == Vocabulary::UNK_ID && token.text != Vocabulary::UNK_TOKEN) {
                count++;
            }
        }
    }
    return count;
}

size_t StatisticsAnalyzer::countUniqueUnknownTokens(const Dataset& dataset, const Vocabulary& vocab) {
    std::unordered_set<std::string> unknownSet;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            int id = vocab.getTokenId(token.text);
            if (id == Vocabulary::UNK_ID && token.text != Vocabulary::UNK_TOKEN) {
                unknownSet.insert(token.text);
            }
        }
    }
    return unknownSet.size();
}

double StatisticsAnalyzer::calculateAverageTokenLength(const Dataset& dataset) {
    size_t totalTokens = countTotalTokens(dataset);
    if (totalTokens == 0) {
        return 0.0;
    }
    return static_cast<double>(countTokenCharacters(dataset)) / totalTokens;
}

std::unordered_map<std::string, size_t> StatisticsAnalyzer::countTokenFrequencies(const Dataset& dataset) {
    std::unordered_map<std::string, size_t> frequencies;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            frequencies[token.text]++;
        }
    }
    return frequencies;
}

std::unordered_map<std::string, size_t> StatisticsAnalyzer::countDocumentFrequencies(const Dataset& dataset) {
    std::unordered_map<std::string, size_t> docFrequencies;
    for (const auto& doc : dataset.getDocuments()) {
        std::unordered_set<std::string> seenInDoc;
        for (const auto& token : doc.tokens) {
            seenInDoc.insert(token.text);
        }
        for (const auto& text : seenInDoc) {
            docFrequencies[text]++;
        }
    }
    return docFrequencies;
}

std::vector<TokenFrequencyInfo> StatisticsAnalyzer::getTokenFrequencyTable(
    const Dataset& dataset,
    const Vocabulary* vocab,
    size_t topN) {

    size_t totalTokens = countTotalTokens(dataset);
    auto frequencies = countTokenFrequencies(dataset);
    auto docFrequencies = countDocumentFrequencies(dataset);

    std::unordered_map<std::string, TokenType> tokenTypes;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            if (tokenTypes.find(token.text) == tokenTypes.end()) {
                tokenTypes[token.text] = token.type;
            }
        }
    }

    std::vector<TokenFrequencyInfo> table;
    table.reserve(frequencies.size());

    for (const auto& [text, count] : frequencies) {
        TokenFrequencyInfo info;
        info.text = text;
        info.type = tokenTypes[text];
        info.count = count;
        info.documentFrequency = docFrequencies[text];
        info.tokenId = (vocab != nullptr) ? vocab->getTokenId(text) : -1;

        if (totalTokens > 0) {
            info.relativeFrequency = static_cast<double>(count) / totalTokens;
            info.percentage = info.relativeFrequency * 100.0;
        }

        table.push_back(info);
    }

    std::sort(table.begin(), table.end(),
              [](const TokenFrequencyInfo& a, const TokenFrequencyInfo& b) {
                  if (a.count != b.count) {
                      return a.count > b.count;
                  }
                  return a.text < b.text;
              });

    if (topN > 0 && table.size() > topN) {
        table.resize(topN);
    }

    return table;
}

std::unordered_map<TokenType, size_t> StatisticsAnalyzer::countTokensByType(const Dataset& dataset) {
    std::unordered_map<TokenType, size_t> counts = {
        {TokenType::Word, 0},
        {TokenType::Number, 0},
        {TokenType::Punctuation, 0},
        {TokenType::Special, 0},
        {TokenType::Whitespace, 0},
        {TokenType::Unknown, 0}
    };

    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            counts[token.type]++;
        }
    }
    return counts;
}

std::vector<TokenTypeStats> StatisticsAnalyzer::getTokenTypeDistribution(const Dataset& dataset) {
    const std::vector<TokenType> orderedTypes = {
        TokenType::Word,
        TokenType::Number,
        TokenType::Punctuation,
        TokenType::Special,
        TokenType::Whitespace,
        TokenType::Unknown
    };

    size_t totalTokens = countTotalTokens(dataset);
    std::unordered_map<TokenType, size_t> totalCounts;
    std::unordered_map<TokenType, size_t> charSums;
    std::unordered_map<TokenType, std::unordered_set<std::string>> uniqueSets;

    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            totalCounts[token.type]++;
            charSums[token.type] += token.text.size();
            uniqueSets[token.type].insert(token.text);
        }
    }

    std::vector<TokenTypeStats> result;
    result.reserve(orderedTypes.size());

    for (TokenType type : orderedTypes) {
        TokenTypeStats item;
        item.type = type;
        item.typeName = tokenTypeToString(type);
        item.totalCount = totalCounts[type];
        item.uniqueCount = uniqueSets[type].size();

        if (totalTokens > 0) {
            item.percentage = (static_cast<double>(item.totalCount) / totalTokens) * 100.0;
        }
        if (item.totalCount > 0) {
            item.avgLength = static_cast<double>(charSums[type]) / item.totalCount;
        }

        result.push_back(item);
    }

    return result;
}

void StatisticsAnalyzer::printReport(const CorpusStatistics& stats, const std::string& title) {
    std::cout << "=== " << title << " ===\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Documents count            : " << stats.documentCount << "\n";
    std::cout << "Total raw characters       : " << stats.totalRawCharacters << "\n";
    std::cout << "Total token characters     : " << stats.totalTokenCharacters << "\n";
    std::cout << "Total tokens (T)           : " << stats.totalTokens << "\n";
    std::cout << "Unique corpus tokens       : " << stats.uniqueTokens << "\n";
    std::cout << "In-vocabulary unique tokens: " << stats.inVocabUniqueTokens << "\n";
    std::cout << "Unknown token occurrences  : " << stats.unknownTokenOccurrences
              << " (" << std::setprecision(2) << (stats.unknownTokenRatio * 100.0) << "%)\n";
    std::cout << std::setprecision(4);
    std::cout << "Unique unknown tokens      : " << stats.uniqueUnknownTokens << "\n";
    std::cout << "Avg doc length (chars)     : " << stats.avgDocumentLengthChars << "\n";
    std::cout << "Avg doc length (tokens)    : " << stats.avgDocumentLengthTokens << "\n";
    std::cout << "Avg token length (all)     : " << stats.avgTokenLength << "\n";
    std::cout << "Avg token length (unique)  : " << stats.avgUniqueTokenLength << "\n";
    std::cout << "Avg unknown token length   : " << stats.avgUnknownTokenLength << "\n\n";
}

void StatisticsAnalyzer::printTokenFrequencyTable(const std::vector<TokenFrequencyInfo>& table,
                                                 const std::string& title) {
    std::cout << "=== " << title << " ===\n";
    std::cout << std::left
              << std::setw(14) << "Token"
              << std::setw(8)  << "ID"
              << std::setw(14) << "Type"
              << std::setw(8)  << "Count"
              << std::setw(8)  << "DocFreq"
              << std::setw(10) << "RelFreq"
              << "Share (%)\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& item : table) {
        std::cout << std::left
                  << std::setw(14) << item.text
                  << std::setw(8)  << item.tokenId
                  << std::setw(14) << tokenTypeToString(item.type)
                  << std::setw(8)  << item.count
                  << std::setw(8)  << item.documentFrequency
                  << std::fixed << std::setprecision(4)
                  << std::setw(10) << item.relativeFrequency
                  << std::setprecision(2) << item.percentage << "%\n";
    }
    std::cout << "\n";
}

void StatisticsAnalyzer::printTokenTypeDistribution(const std::vector<TokenTypeStats>& distribution,
                                                   const std::string& title) {
    std::cout << "=== " << title << " ===\n";
    std::cout << std::left
              << std::setw(15) << "Token Type"
              << std::setw(12) << "Total"
              << std::setw(12) << "Unique"
              << std::setw(12) << "Share (%)"
              << "Avg Length\n";
    std::cout << std::string(62, '-') << "\n";

    for (const auto& item : distribution) {
        std::cout << std::left
                  << std::setw(15) << item.typeName
                  << std::setw(12) << item.totalCount
                  << std::setw(12) << item.uniqueCount
                  << std::fixed << std::setprecision(2)
                  << std::setw(12) << item.percentage
                  << std::setprecision(4) << item.avgLength << "\n";
    }
    std::cout << "\n";
}