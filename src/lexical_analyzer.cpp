#include "lexical_analyzer.h"
#include <unordered_set>
#include <string>

std::size_t LexicalAnalyzer::countTotalWords(const Text & text) const {
    std::size_t totalWords = 0;

    // Якщо Text — це клас із геттером, замініть text на text.getTokens()
    for (const auto& token : text) {
        if (token.getType() == TokenType::WORD) {
            ++totalWords;
        }
    }

    return totalWords;
}

std::size_t LexicalAnalyzer::countUniqueWords(const Text& text) const {
    std::unordered_set<std::string> uniqueLemmas;

    for (const auto& token : text) {
        if (token.getType() == TokenType::WORD) {
            std::string form = token.getLemma();

            // Якщо лематизація ще не проводилася, беремо normalizedText
            if (form.empty()) {
                form = token.getNormalizedText();
            }

            uniqueLemmas.insert(form);
        }
    }

    return uniqueLemmas.size();
}

double LexicalAnalyzer::calculateTTR(const Text& text) const {
    std::size_t totalWords = countTotalWords(text);
    if (totalWords == 0) {
        return 0.0;
    }

    std::size_t uniqueWords = countUniqueWords(text);
    return static_cast<double>(uniqueWords) / static_cast<double>(totalWords);
}