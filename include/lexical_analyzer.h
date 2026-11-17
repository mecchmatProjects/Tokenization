#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <cstddef>
// Підключіть ваш заголовний файл, де визначені Text, Token та TokenType
// #include "text.h" 

class LexicalAnalyzer {
public:
    LexicalAnalyzer() = default;
    ~LexicalAnalyzer() = default;

    // Підрахунок загальної кількості слів (N)
    std::size_t countTotalWords(const Text& text) const;

    // Підрахунок кількості унікальних слів за лемами (V)
    std::size_t countUniqueWords(const Text& text) const;

    // Розрахунок лексичного різноманіття (Type-Token Ratio: V / N)
    double calculateTTR(const Text& text) const;
};

#endif // LEXICAL_ANALYZER_H