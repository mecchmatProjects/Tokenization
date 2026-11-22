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
    Token t1("Кіт", "кіт", TokenType::WORD, 0, 1);
    t1.setLemma("кіт");

    Token t2("їсть", "їсть", TokenType::WORD, 1, 1);
    t2.setLemma("їсти");

    Token t3(".", ".", TokenType::PUNCTUATION, 2, 1);
    t3.setLemma(".");

    Token t4("Кіт", "кіт", TokenType::WORD, 3, 2);
    t4.setLemma("кіт");

    Token t5("спить", "спить", TokenType::WORD, 4, 2);
    t5.setLemma("спати");

    Token t6(".", ".", TokenType::PUNCTUATION, 5, 2);
    t6.setLemma(".");

    Text sampleText = { t1, t2, t3, t4, t5, t6 };

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
    // 1. Тест для тексту лише зі знаків пунктуації
    Text punctuationOnlyText = {
        Token("...", "...", TokenType::PUNCTUATION, 0, 1),
        Token("!",   "!",   TokenType::PUNCTUATION, 1, 1)
    };

    LexicalAnalyzer analyzer;

    assert(analyzer.countTotalWords(punctuationOnlyText) == 0);
    assert(analyzer.countUniqueWords(punctuationOnlyText) == 0);
    assert(isAlmostEqual(analyzer.calculateTTR(punctuationOnlyText), 0.0));

    // 2. Тест для абсолютно порожнього вектора токенів
    Text completelyEmptyText;

    assert(analyzer.countTotalWords(completelyEmptyText) == 0);
    assert(analyzer.countUniqueWords(completelyEmptyText) == 0);
    assert(isAlmostEqual(analyzer.calculateTTR(completelyEmptyText), 0.0));

    std::cout << "[PASS] testEmptyOrPunctuationOnlyText\n";
}

int main() {
    testStandardPhrase();
    testEmptyOrPunctuationOnlyText();

    std::cout << "All LexicalAnalyzer tests passed!\n";
    return 0;
}
