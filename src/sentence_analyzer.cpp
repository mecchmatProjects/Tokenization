#include "sentence_analyzer.h"
#include <algorithm>
#include <numeric>

bool SentenceAnalyzer::isSentenceEndingPunctuation(const std::string& tokenText) const {
    return tokenText == "." || tokenText == "!" || tokenText == "?" ||
        tokenText == "..." || tokenText == "…";
}

std::vector<std::size_t> SentenceAnalyzer::extractSentenceLengths(const Text& text) const {
    std::vector<std::size_t> lengths;
    std::size_t currentWordCount = 0;

    for (const auto& token : text.tokens) {
        if (token.type == TokenType::WORD) {
            ++currentWordCount;
        }
        else if (token.type == TokenType::PUNCTUATION && isSentenceEndingPunctuation(token.word)) {
            // Фіксуємо довжину речення, якщо в ньому було хоча б одне слово
            if (currentWordCount > 0) {
                lengths.push_back(currentWordCount);
                currentWordCount = 0;
            }
        }
    }

    // Якщо текст закінчився без крапки в кінці, але слова були — враховуємо останнє речення
    if (currentWordCount > 0) {
        lengths.push_back(currentWordCount);
    }

    return lengths;
}

std::size_t SentenceAnalyzer::countSentences(const Text& text) const {
    return extractSentenceLengths(text).size();
}

double SentenceAnalyzer::calculateAverageSentenceLength(const Text& text) const {
    std::vector<std::size_t> lengths = extractSentenceLengths(text);
    if (lengths.empty()) {
        return 0.0;
    }

    std::size_t totalWords = std::accumulate(lengths.begin(), lengths.end(), std::size_t{ 0 });
    return static_cast<double>(totalWords) / static_cast<double>(lengths.size());
}

std::size_t SentenceAnalyzer::findMaxSentenceLength(const Text& text) const {
    std::vector<std::size_t> lengths = extractSentenceLengths(text);
    if (lengths.empty()) {
        return 0;
    }

    return *std::max_element(lengths.begin(), lengths.end());
}

double SentenceAnalyzer::calculateMedianSentenceLength(const Text& text) const {
    std::vector<std::size_t> lengths = extractSentenceLengths(text);
    if (lengths.empty()) {
        return 0.0;
    }

    std::sort(lengths.begin(), lengths.end());
    std::size_t mid = lengths.size() / 2;

    // Якщо кількість речень парна — беремо середнє двох центральних значень
    if (lengths.size() % 2 == 0) {
        return static_cast<double>(lengths[mid - 1] + lengths[mid]) / 2.0;
    }

    // Якщо непарна — повертаємо центральний елемент
    return static_cast<double>(lengths[mid]);
}

std::size_t SentenceAnalyzer::countTotalPunctuation(const Text& text) const {
    std::size_t count = 0;
    for (const auto& token : text.tokens) {
        if (token.type == TokenType::PUNCTUATION) {
            ++count;
        }
    }
    return count;
}

std::unordered_map<std::string, std::size_t> SentenceAnalyzer::analyzePunctuationFrequency(const Text& text) const {
    std::unordered_map<std::string, std::size_t> frequency;

    for (const auto& token : text.tokens) {
        if (token.type == TokenType::PUNCTUATION) {
            // Використовуємо базове текстове представлення знаку (token.word або token.lemma)
            ++frequency[token.word];
        }
    }

    return frequency;
}