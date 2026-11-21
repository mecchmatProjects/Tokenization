#include "style_profiler.h"
#include <cmath> // для std::sqrt

namespace litcritic {

    StyleProfile StyleProfiler::buildProfile(
        const TextDocument& document,
        const ProfilerConfig& config
    ) {
        StyleProfile profile;
        const auto& sentences = document.getSentences();

        // Збираємо всі токени з речень у формат Text (std::vector<Token>) для LexicalAnalyzer
        Text allTokens;
        double totalWordChars = 0.0;
        int wordTokenCount = 0;

        for (const auto& sentence : sentences) {
            const auto& sentTokens = sentence.getTokens();
            allTokens.insert(allTokens.end(), sentTokens.begin(), sentTokens.end());

            for (const auto& token : sentTokens) {
                if (token.getType() == TokenType::WORD) {
                    totalWordChars += static_cast<double>(token.getText().length());
                    wordTokenCount++;
                }
            }
        }

        // 1. Лексичний аналіз через об'єкт LexicalAnalyzer
        LexicalAnalyzer lexicalAnalyzer;
        const int totalWords = static_cast<int>(lexicalAnalyzer.countTotalWords(allTokens));
        const int uniqueWords = static_cast<int>(lexicalAnalyzer.countUniqueWords(allTokens));
        const int lemmaCount = uniqueWords; // Поки що використовуємо кількість унікальних слів
        const double lexicalDiversity = lexicalAnalyzer.calculateTTR(allTokens);
        const double avgWordLength = (wordTokenCount > 0)
            ? (totalWordChars / static_cast<double>(wordTokenCount))
            : 0.0;

        profile.setLexicalStats(
            totalWords,
            uniqueWords,
            lemmaCount,
            lexicalDiversity,
            avgWordLength
        );

        // 2. Синтаксичний аналіз речень (середня довжина та стандартне відхилення)
        // Якщо SentenceAnalyzer::calculateAverageSentenceLength теж нестатичний,
        // створи його об'єкт: SentenceAnalyzer sentenceAnalyzer;

        SentenceAnalyzer sentenceAnalyzer;
        const double avgSentLen = sentenceAnalyzer.calculateAverageSentenceLength(allTokens);

        double varianceSum = 0.0;
        for (const auto& sentence : sentences) {
            const double diff = static_cast<double>(sentence.getWordCount()) - avgSentLen;
            varianceSum += diff * diff;
        }
        const double stDev = (!sentences.empty())
            ? std::sqrt(varianceSum / static_cast<double>(sentences.size()))
            : 0.0;

        profile.setSentenceStats(avgSentLen, stDev);

        // 3. Частотний аналіз та N-грами
        const auto wordFrequencies = NGramAnalyzer::countWordFrequencies(document);
        const auto topWords = NGramAnalyzer::getTopNGrams(wordFrequencies, config.topWordsLimit);
        for (const auto& item : topWords) {
            profile.addNgramStat(item.first, static_cast<int>(item.second));
        }

        const auto bigrams = NGramAnalyzer::extractBigrams(sentences);
        auto repeatedBigrams = NGramAnalyzer::getRepeatedPhrases(bigrams, config.minPhraseRepetitions);
        if (repeatedBigrams.size() > config.topNGramsLimit) {
            repeatedBigrams.resize(config.topNGramsLimit);
        }
        for (const auto& item : repeatedBigrams) {
            profile.addNgramStat(item.first, static_cast<int>(item.second));
        }

        const auto trigrams = NGramAnalyzer::extractTrigrams(sentences);
        auto repeatedTrigrams = NGramAnalyzer::getRepeatedPhrases(trigrams, config.minPhraseRepetitions);
        if (repeatedTrigrams.size() > config.topNGramsLimit) {
            repeatedTrigrams.resize(config.topNGramsLimit);
        }
        for (const auto& item : repeatedTrigrams) {
            profile.addNgramStat(item.first, static_cast<int>(item.second));
        }

        // 4. Індекс повторюваності
        const double repetitionIndex = (totalWords > 0)
            ? static_cast<double>(repeatedBigrams.size() + repeatedTrigrams.size()) / totalWords
            : 0.0;
        profile.setRepetitionIndex(repetitionIndex);

        return profile;
    }

} // namespace litcritic

