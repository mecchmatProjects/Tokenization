#include "sentence_analyzer.h"
#include <algorithm>
#include <map>
#include <numeric>

std::vector<std::size_t> SentenceAnalyzer::extractSentenceLengths(const Text& text) const {
    // Використовуємо std::map (ключ: sentenceNumber, значення: кількість слів у цьому реченні)
    // Враховуються лише речення, що містять хоча б одне слово (TokenType::WORD)
    std::map<int, std::size_t> wordsPerSentence;

    for (const auto& token : text) {
        if (token.getType() == TokenType::WORD) {
            ++wordsPerSentence[token.getSentenceNumber()];
        }
    }

    std::vector<std::size_t> lengths;
    lengths.reserve(wordsPerSentence.size());

    for (const auto& pair : wordsPerSentence) {
        lengths.push_back(pair.second);
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

    if (lengths.size() % 2 == 0) {
        return static_cast<double>(lengths[mid - 1] + lengths[mid]) / 2.0;
    }

    return static_cast<double>(lengths[mid]);
}

std::size_t SentenceAnalyzer::countTotalPunctuation(const Text& text) const {
    std::size_t count = 0;

    for (const auto& token : text) {
        if (token.getType() == TokenType::PUNCTUATION) {
            ++count;
        }
    }

    return count;
}

std::unordered_map<std::string, std::size_t> SentenceAnalyzer::analyzePunctuationFrequency(const Text& text) const {
    std::unordered_map<std::string, std::size_t> frequency;

    for (const auto& token : text) {
        if (token.getType() == TokenType::PUNCTUATION) {
            ++frequency[token.getText()];
        }
    }

    return frequency;
}