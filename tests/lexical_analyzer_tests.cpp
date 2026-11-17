#include "lexical_analyzer.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

// Допоміжна функція для порівняння чисел з плаваючою комою
bool isAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void testStandardPhrase() {
    // 1. Формуємо фейковий текст для фрази: "Кіт їсть. Кіт спить."
    Text sampleText;
    sampleText.tokens = {
        { "Кіт",   "кіт",   TokenType::WORD },
        { "їсть",  "їсти",  TokenType::WORD },
        { ".",     ".",     TokenType::PUNCTUATION },
        { "Кіт",   "кіт",   TokenType::WORD },
        { "спить", "спати", TokenType::WORD },
        { ".",     ".",     TokenType::PUNCTUATION }
    };

    LexicalAnalyzer analyzer;

    // 2. Перевіряємо загальну кількість слів (має бути 4, крапки ігноруються)
    std::size_t total = analyzer.countTotalWords(sampleText);
    assert(total == 4 && "Error: total words should be 4");

    // 3. Перевіряємо кількість унікальних лем (має бути 3: "кіт", "їсти", "спати")
    std::size_t unique = analyzer.countUniqueWords(sampleText);
    assert(unique == 3 && "Error: unique words should be 3");

    // 4. Перевіряємо TTR (3 / 4 = 0.75)
    double ttr = analyzer.calculateTTR(sampleText);
    assert(isAlmostEqual(ttr, 0.75) && "Error: TTR should be 0.75");

    std::cout << "[PASS] testStandardPhrase\n";
}

void testEmptyOrPunctuationOnlyText() {
    Text emptyText;
    emptyText.tokens = {
        { "...", "...", TokenType::PUNCTUATION },
        { "!",   "!",   TokenType::PUNCTUATION }
    };

    LexicalAnalyzer analyzer;

    assert(analyzer.countTotalWords(emptyText) == 0);
    assert(analyzer.countUniqueWords(emptyText) == 0);
    assert(isAlmostEqual(analyzer.calculateTTR(emptyText), 0.0));

    std::cout << "[PASS] testEmptyOrPunctuationOnlyText\n";
}

int main() {
    testStandardPhrase();
    testEmptyOrPunctuationOnlyText();

    std::cout << "All LexicalAnalyzer tests passed!\n";
    return 0;
}