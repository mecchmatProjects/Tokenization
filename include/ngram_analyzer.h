#ifndef LITCRITIC_NGRAM_ANALYZER_H
#define LITCRITIC_NGRAM_ANALYZER_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "sentence.h"
#include "text_document.h"
#include "token.h"

namespace litcritic {

    using FrequencyMap = std::unordered_map<std::string, std::size_t>;
    using RankedNGram = std::pair<std::string, std::size_t>;

    class NGramAnalyzer {
    public:
        // Підрахунок частоти окремих слів (уніграм)
        static FrequencyMap countWordFrequencies(const std::vector<Token>& tokens);
        static FrequencyMap countWordFrequencies(const TextDocument& document);

        // Загальний метод для пошуку N-грам довільної довжини (n = 1, 2, 3...)
        // Враховує межі речень, щоб не склеювати кінець одного речення з початком іншого
        static FrequencyMap extractNGrams(const std::vector<Sentence>& sentences, std::size_t n);
        static FrequencyMap extractNGrams(const TextDocument& document, std::size_t n);

        // Зручні обгортки для біграм (n = 2) та триграм (n = 3)
        static FrequencyMap extractBigrams(const std::vector<Sentence>& sentences);
        static FrequencyMap extractTrigrams(const std::vector<Sentence>& sentences);

        // Повертає лише ті N-грами, що зустрічаються >= minCount разів, відсортовані за спаданням частоти
        static std::vector<RankedNGram> getRepeatedPhrases(
            const FrequencyMap& ngrams,
            std::size_t minCount = 2
        );

        // Повертає топ-K найчастіших слів або фраз
        static std::vector<RankedNGram> getTopNGrams(
            const FrequencyMap& ngrams,
            std::size_t topK = 10
        );

    private:
        static std::vector<std::string> extractCleanWords(const std::vector<Token>& tokens);
        static std::string toLowerAscii(const std::string& str);
    };

} // namespace litcritic

#endif // LITCRITIC_NGRAM_ANALYZER_H