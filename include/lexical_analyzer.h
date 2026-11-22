#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <cstddef>
// Підключіть ваш заголовний файл, де визначені Text, Token та TokenType
#include <vector>
#include "token.h"

using Text = std::vector<Token>;

class LexicalAnalyzer {
public:
    LexicalAnalyzer() = default;
    ~LexicalAnalyzer() = default;

    std::size_t countTotalWords(const Text& text) const;
    std::size_t countUniqueWords(const Text& text) const;

    double calculateTTR(const Text& text) const;
};

#endif // LEXICAL_ANALYZER_H