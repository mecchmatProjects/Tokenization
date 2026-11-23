#include "text_comparator.h"

#include <algorithm>
#include <cmath>

namespace litcritic {

    StyleComparisonResult TextComparator::compare(
        const StyleProfile& first,
        const StyleProfile& second,
        const ComparisonWeights& weights
    ) {
        StyleComparisonResult result;

        // 1. Базові різниці метрик
        result.wordCountDiff = std::abs(first.getTotalWords() - second.getTotalWords());
        result.lexicalDiversityDiff = std::abs(
            first.getLexicalDiversity() - second.getLexicalDiversity()
        );
        result.averageSentenceLengthDiff = std::abs(
            first.getAverageSentenceLength() - second.getAverageSentenceLength()
        );

        // 2. Нормалізація різниць до шкали [0.0, 1.0]
        // lexicalDiversity вже лежить у [0.0, 1.0]
        const double normLexicalDiff = std::min(1.0, result.lexicalDiversityDiff);

        const double scale = (weights.maxExpectedSentenceLengthDiff > 0.0)
            ? weights.maxExpectedSentenceLengthDiff
            : 25.0;
        const double normSentenceDiff = std::min(1.0, result.averageSentenceLengthDiff / scale);

        // 3. Зважена евклідова відстань
        const double totalWeight = weights.lexicalDiversityWeight + weights.sentenceLengthWeight;
        if (totalWeight > 0.0) {
            const double wLex = weights.lexicalDiversityWeight / totalWeight;
            const double wSent = weights.sentenceLengthWeight / totalWeight;

            const double weightedSqSum =
                wLex * (normLexicalDiff * normLexicalDiff) +
                wSent * (normSentenceDiff * normSentenceDiff);

            result.normalizedDistance = std::sqrt(weightedSqSum);
        }
        else {
            result.normalizedDistance = 0.0;
        }

        // 4. Перетворення відстані на коефіцієнт схожості [0.0, 1.0]
        result.similarityScore = std::max(0.0, 1.0 - result.normalizedDistance);

        return result;
    }

    double TextComparator::calculateCosineSimilarity(
        const std::map<std::string, int>& dictA,
        const std::map<std::string, int>& dictB
    ) {
        if (dictA.empty() || dictB.empty()) {
            return 0.0;
        }

        double dotProduct = 0.0;
        double normA = 0.0;
        double normB = 0.0;

        for (const auto& entry : dictA) {
            const double valA = static_cast<double>(entry.second);
            normA += valA * valA;

            const auto it = dictB.find(entry.first);
            if (it != dictB.end()) {
                dotProduct += valA * static_cast<double>(it->second);
            }
        }

        for (const auto& entry : dictB) {
            const double valB = static_cast<double>(entry.second);
            normB += valB * valB;
        }

        const double denominator = std::sqrt(normA) * std::sqrt(normB);
        if (denominator == 0.0) {
            return 0.0;
        }

        return dotProduct / denominator;
    }

} // namespace litcritic