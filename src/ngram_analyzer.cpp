#include "ngram_analyzer.h"

#include <algorithm>
#include <cctype>

namespace litcritic {

    std::string NGramAnalyzer::toLowerAscii(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    std::vector<std::string> NGramAnalyzer::extractCleanWords(const std::vector<Token>& tokens) {
        std::vector<std::string> words;
        words.reserve(tokens.size());

        for (const auto& token : tokens) {
            // Адаптуйте перевірку під вашу структуру Token (наприклад, token.getType() == TokenType::Word)
            if (token.getType() == TokenType::WORD && !token.getText().empty()) {
                words.push_back(toLowerAscii(token.getText()));
            }
        }
        return words;
    }

    FrequencyMap NGramAnalyzer::countWordFrequencies(const std::vector<Token>& tokens) {
        FrequencyMap frequencies;
        const auto words = extractCleanWords(tokens);

        for (const auto& word : words) {
            ++frequencies[word];
        }
        return frequencies;
    }

    FrequencyMap NGramAnalyzer::countWordFrequencies(const TextDocument& document) {
        FrequencyMap totalFrequencies;
        for (const auto& sentence : document.getSentences()) {
            const auto words = extractCleanWords(sentence.getTokens());
            for (const auto& word : words) {
                ++totalFrequencies[word];
            }
        }
        return totalFrequencies;
    }

    FrequencyMap NGramAnalyzer::extractNGrams(const std::vector<Sentence>& sentences, std::size_t n) {
        FrequencyMap ngrams;
        if (n == 0) {
            return ngrams;
        }

        for (const auto& sentence : sentences) {
            const auto words = extractCleanWords(sentence.getTokens());
            if (words.size() < n) {
                continue;
            }

            for (std::size_t i = 0; i <= words.size() - n; ++i) {
                std::string phrase = words[i];
                for (std::size_t j = 1; j < n; ++j) {
                    phrase += " " + words[i + j];
                }
                ++ngrams[phrase];
            }
        }
        return ngrams;
    }

    FrequencyMap NGramAnalyzer::extractNGrams(const TextDocument& document, std::size_t n) {
        return extractNGrams(document.getSentences(), n);
    }

    FrequencyMap NGramAnalyzer::extractBigrams(const std::vector<Sentence>& sentences) {
        return extractNGrams(sentences, 2);
    }

    FrequencyMap NGramAnalyzer::extractTrigrams(const std::vector<Sentence>& sentences) {
        return extractNGrams(sentences, 3);
    }

    std::vector<RankedNGram> NGramAnalyzer::getRepeatedPhrases(
        const FrequencyMap& ngrams,
        std::size_t minCount
    ) {
        std::vector<RankedNGram> repeated;
        for (const auto& entry : ngrams) {
            if (entry.second >= minCount) {
                repeated.push_back(entry);
            }
        }

        // Сортування: спочатку за спаданням частоти, при рівній частоті — за алфавітом
        std::sort(repeated.begin(), repeated.end(),
            [](const RankedNGram& a, const RankedNGram& b) {
                if (a.second != b.second) {
                    return a.second > b.second;
                }
                return a.first < b.first;
            });

        return repeated;
    }

    std::vector<RankedNGram> NGramAnalyzer::getTopNGrams(
        const FrequencyMap& ngrams,
        std::size_t topK
    ) {
        auto ranked = getRepeatedPhrases(ngrams, 1);
        if (ranked.size() > topK) {
            ranked.resize(topK);
        }
        return ranked;
    }

} // namespace litcritic