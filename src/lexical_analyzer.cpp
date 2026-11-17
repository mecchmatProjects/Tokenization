#include "lexical_analyzer.h"
#include <unordered_set>
#include <string>

std::size_t LexicalAnalyzer::countTotalWords(const Text& text) const {
    std::size_t totalWords = 0;

    // Якщо у Text немає прямого доступу до поля tokens, замініть на text.getTokens()
    for (const auto& token : text.tokens) {
        if (token.type == TokenType::WORD) {
            ++totalWords;
        }
    }

    return totalWords;
}

std::size_t LexicalAnalyzer::countUniqueWords(const Text& text) const {
    std::unordered_set<std::string> uniqueLemmas;

    for (const auto& token : text.tokens) {
        if (token.type == TokenType::WORD) {
            // Закидаємо лему в set; дублікати ігноруються автоматично
            uniqueLemmas.insert(token.lemma);
        }
    }

    return uniqueLemmas.size();
}

double LexicalAnalyzer::calculateTTR(const Text& text) const {
    std::size_t totalWords = countTotalWords(text);

    // Захист від ділення на нуль для порожніх текстів
    if (totalWords == 0) {
        return 0.0;
    }

    std::size_t uniqueWords = countUniqueWords(text);

    // Обов'язково приводимо до double, інакше 3 / 4 дасть 0
    return static_cast<double>(uniqueWords) / static_cast<double>(totalWords);
}