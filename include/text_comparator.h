#ifndef LITCRITIC_TEXT_COMPARATOR_H
#define LITCRITIC_TEXT_COMPARATOR_H

#include <map>
#include <string>

#include "style_profile.h"

namespace litcritic {

    struct StyleComparisonResult {
        // Абсолютні різниці ключових метрик
        int wordCountDiff = 0;
        double lexicalDiversityDiff = 0.0;
        double averageSentenceLengthDiff = 0.0;

        // Нормалізована евклідова відстань між векторами стилю (>= 0.0, 0.0 = ідентичні)
        double normalizedDistance = 0.0;

        // Підсумковий індекс схожості стилів у діапазоні [0.0, 1.0] (де 1.0 = 100% схожості)
        double similarityScore = 1.0;
    };

    struct ComparisonWeights {
        double lexicalDiversityWeight = 0.5;
        double sentenceLengthWeight = 0.5;
        // Очікувана максимальна різниця довжини речень для нормалізації в діапазон [0, 1]
        double maxExpectedSentenceLengthDiff = 25.0;
    };

    class TextComparator {
    public:
        // Комплексне порівняння двох профілів із заданими вагами метрик
        static StyleComparisonResult compare(
            const StyleProfile& first,
            const StyleProfile& second,
            const ComparisonWeights& weights = ComparisonWeights{}
        );

        // Косинусна подібність для порівняння частотних словників (N-грами, пунктуація, частини мови)
        static double calculateCosineSimilarity(
            const std::map<std::string, int>& dictA,
            const std::map<std::string, int>& dictB
        );
    };

} // namespace litcritic

#endif // LITCRITIC_TEXT_COMPARATOR_H