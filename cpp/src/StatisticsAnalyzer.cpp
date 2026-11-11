#include "../include/StatisticsAnalyzer.h"
#include <unordered_set>

bool StatisticsAnalyzer::isTokenUnknown(const Token& token, const Vocabulary* vocab) {
    if (token.type == TokenType::Unknown || token.text == Vocabulary::UNK_TOKEN) {
        return true;
    }
    if (vocab != nullptr) {
        return vocab->getTokenId(token.text) == Vocabulary::UNK_ID;
    }
    return token.tokenId == Vocabulary::UNK_ID;
}

size_t StatisticsAnalyzer::countDocuments(const Dataset& dataset) {
    return dataset.size();
}

size_t StatisticsAnalyzer::countTotalRawCharacters(const Dataset& dataset) {
    size_t totalChars = 0;
    for (const auto& doc : dataset.getDocuments()) {
        if (!doc.rawText.empty()) {
            totalChars += doc.rawText.size();
        } else {
            for (const auto& token : doc.tokens) {
                totalChars += token.text.size();
            }
        }
    }
    return totalChars;
}

size_t StatisticsAnalyzer::countTotalTokenCharacters(const Dataset& dataset) {
    size_t totalTokenChars = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            totalTokenChars += token.text.size();
        }
    }
    return totalTokenChars;
}

size_t StatisticsAnalyzer::countTotalTokens(const Dataset& dataset) {
    size_t totalTokens = 0;
    for (const auto& doc : dataset.getDocuments()) {
        totalTokens += doc.tokens.size();
    }
    return totalTokens;
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

size_t StatisticsAnalyzer::countUnknownTokens(const Dataset& dataset, const Vocabulary* vocab) {
    size_t unknownCount = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            if (isTokenUnknown(token, vocab)) {
                unknownCount++;
            }
        }
    }
    return unknownCount;
}

size_t StatisticsAnalyzer::countUniqueUnknownTokens(const Dataset& dataset, const Vocabulary* vocab) {
    std::unordered_set<std::string> unknownSet;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            if (isTokenUnknown(token, vocab)) {
                unknownSet.insert(token.text);
            }
        }
    }
    return unknownSet.size();
}

double StatisticsAnalyzer::calculateAverageTokenLength(const Dataset& dataset) {
    size_t totalTokens = 0;
    size_t totalChars = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            totalTokens++;
            totalChars += token.text.size();
        }
    }
    if (totalTokens == 0) {
        return 0.0;
    }
    return static_cast<double>(totalChars) / static_cast<double>(totalTokens);
}

double StatisticsAnalyzer::calculateAverageUniqueTokenLength(const Dataset& dataset) {
    std::unordered_set<std::string> uniqueSet;
    size_t uniqueChars = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            auto inserted = uniqueSet.insert(token.text);
            if (inserted.second) {
                uniqueChars += token.text.size();
            }
        }
    }
    if (uniqueSet.empty()) {
        return 0.0;
    }
    return static_cast<double>(uniqueChars) / static_cast<double>(uniqueSet.size());
}

double StatisticsAnalyzer::calculateAverageUnknownTokenLength(const Dataset& dataset, const Vocabulary* vocab) {
    size_t unknownCount = 0;
    size_t unknownChars = 0;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            if (isTokenUnknown(token, vocab)) {
                unknownCount++;
                unknownChars += token.text.size();
            }
        }
    }
    if (unknownCount == 0) {
        return 0.0;
    }
    return static_cast<double>(unknownChars) / static_cast<double>(unknownCount);
}

CorpusStatistics StatisticsAnalyzer::analyze(const Dataset& dataset, const Vocabulary* vocab) {
    CorpusStatistics stats;
    stats.documentCount = dataset.size();

    std::unordered_set<std::string> uniqueSet;
    std::unordered_set<std::string> inVocabSet;
    std::unordered_set<std::string> unknownSet;

    size_t uniqueCharSum = 0;
    size_t unknownCharSum = 0;

    for (const auto& doc : dataset.getDocuments()) {
        if (!doc.rawText.empty()) {
            stats.totalRawCharacters += doc.rawText.size();
        } else {
            for (const auto& token : doc.tokens) {
                stats.totalRawCharacters += token.text.size();
            }
        }

        for (const auto& token : doc.tokens) {
            size_t len = token.text.size();
            stats.totalTokens++;
            stats.totalTokenCharacters += len;

            if (uniqueSet.insert(token.text).second) {
                uniqueCharSum += len;
            }

            if (isTokenUnknown(token, vocab)) {
                stats.unknownTokens++;
                unknownCharSum += len;
                unknownSet.insert(token.text);
            } else {
                inVocabSet.insert(token.text);
            }
        }
    }

    stats.uniqueTokens = uniqueSet.size();
    stats.inVocabularyUniqueTokens = inVocabSet.size();
    stats.uniqueUnknownTokens = unknownSet.size();

    if (stats.documentCount > 0) {
        stats.averageDocumentLengthChars =
            static_cast<double>(stats.totalRawCharacters) / static_cast<double>(stats.documentCount);
        stats.averageDocumentLengthTokens =
            static_cast<double>(stats.totalTokens) / static_cast<double>(stats.documentCount);
    }

    if (stats.totalTokens > 0) {
        stats.averageTokenLength =
            static_cast<double>(stats.totalTokenCharacters) / static_cast<double>(stats.totalTokens);
        stats.unknownTokenRatio =
            static_cast<double>(stats.unknownTokens) / static_cast<double>(stats.totalTokens);
    }

    if (stats.uniqueTokens > 0) {
        stats.averageUniqueTokenLength =
            static_cast<double>(uniqueCharSum) / static_cast<double>(stats.uniqueTokens);
    }

    if (stats.unknownTokens > 0) {
        stats.averageUnknownTokenLength =
            static_cast<double>(unknownCharSum) / static_cast<double>(stats.unknownTokens);
    }

    return stats;
}