#ifndef SENTENCE_ANALYZER_H
#define SENTENCE_ANALYZER_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
// #include "text.h"

class SentenceAnalyzer {
public:
    SentenceAnalyzer() = default;
    ~SentenceAnalyzer() = default;

    // 1. Підрахунок загальної кількості речень у тексті
    std::size_t countSentences(const Text& text) const;

    // 2. Розрахунок середньої довжини речення (у словах)
    double calculateAverageSentenceLength(const Text& text) const;

    // 3. Пошук максимальної довжини речення (у словах)
    std::size_t findMaxSentenceLength(const Text& text) const;

    // 4. Розрахунок медіанної довжини речення (у словах)
    double calculateMedianSentenceLength(const Text& text) const;

    // 5. Підрахунок загальної кількості знаків пунктуації
    std::size_t countTotalPunctuation(const Text& text) const;

    // 6. Частотний аналіз кожного знаку пунктуації (наприклад: "." -> 2, "," -> 1)
    std::unordered_map<std::string, std::size_t> analyzePunctuationFrequency(const Text& text) const;

private:
    // Допоміжний метод: повертає вектор довжин (кількості слів) для кожного речення
    std::vector<std::size_t> extractSentenceLengths(const Text& text) const;

    // Допоміжний метод: перевіряє, чи завершує токен речення
    bool isSentenceEndingPunctuation(const std::string& tokenText) const;
};

#endif // SENTENCE_ANALYZER_H