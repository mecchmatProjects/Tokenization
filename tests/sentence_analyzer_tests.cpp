#include "sentence_analyzer.h"
#include <cassert>
#include <cmath>
#include <iostream>

bool isAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void testSentenceMetricsOddCount() {
    // Речення 1 (sentNumber = 1): "Кіт спить." -> 2 слова
    // Речення 2 (sentNumber = 2): "Собака, здається, голосно гавкає на вулиці!" -> 6 слів
    // Речення 3 (sentNumber = 3): "Тиша." -> 1 слово
    Text text = {
        Token("Кіт",      "кіт",      TokenType::WORD,        0,  1),
        Token("спить",    "спить",    TokenType::WORD,        1,  1),
        Token(".",        ".",        TokenType::PUNCTUATION, 2,  1),

        Token("Собака",   "собака",   TokenType::WORD,        3,  2),
        Token(",",        ",",        TokenType::PUNCTUATION, 4,  2),
        Token("здається", "здається", TokenType::WORD,        5,  2),
        Token(",",        ",",        TokenType::PUNCTUATION, 6,  2),
        Token("голосно",  "голосно",  TokenType::WORD,        7,  2),
        Token("гавкає",   "гавкає",   TokenType::WORD,        8,  2),
        Token("на",       "на",       TokenType::WORD,        9,  2),
        Token("вулиці",   "вулиці",   TokenType::WORD,        10, 2),
        Token("!",        "!",        TokenType::PUNCTUATION, 11, 2),

        Token("Тиша",     "тиша",     TokenType::WORD,        12, 3),
        Token(".",        ".",        TokenType::PUNCTUATION, 13, 3)
    };

    SentenceAnalyzer analyzer;

    assert(analyzer.countSentences(text) == 3);
    assert(isAlmostEqual(analyzer.calculateAverageSentenceLength(text), 3.0));
    assert(analyzer.findMaxSentenceLength(text) == 6);
    assert(isAlmostEqual(analyzer.calculateMedianSentenceLength(text), 2.0));

    std::cout << "[PASS] testSentenceMetricsOddCount\n";
}

void testMedianEvenCount() {
    // Речення 1 (sentNumber = 1): 2 слова
    // Речення 2 (sentNumber = 2): 4 слова
    // Медіана: (2 + 4) / 2 = 3.0
    Text text = {
        Token("Йде",     "йде",     TokenType::WORD,        0, 1),
        Token("дощ",     "дощ",     TokenType::WORD,        1, 1),
        Token(".",       ".",       TokenType::PUNCTUATION, 2, 1),

        Token("Сонце",   "сонце",   TokenType::WORD,        3, 2),
        Token("світить", "світить", TokenType::WORD,        4, 2),
        Token("дуже",    "дуже",    TokenType::WORD,        5, 2),
        Token("яскраво", "яскраво", TokenType::WORD,        6, 2),
        Token("!",       "!",       TokenType::PUNCTUATION, 7, 2)
    };

    SentenceAnalyzer analyzer;

    assert(analyzer.countSentences(text) == 2);
    assert(isAlmostEqual(analyzer.calculateMedianSentenceLength(text), 3.0));

    std::cout << "[PASS] testMedianEvenCount\n";
}

void testPunctuationAnalysis() {
    Text text = {
        Token("Привіт", "привіт", TokenType::WORD,        0,  1),
        Token(",",      ",",      TokenType::PUNCTUATION, 1,  1),
        Token("світе",  "світе",  TokenType::WORD,        2,  1),
        Token("!",      "!",      TokenType::PUNCTUATION, 3,  1),
        Token("Як",     "як",     TokenType::WORD,        4,  2),
        Token("справи", "справи", TokenType::WORD,        5,  2),
        Token("?",      "?",      TokenType::PUNCTUATION, 6,  2),
        Token("Добре",  "добре",  TokenType::WORD,        7,  3),
        Token(".",      ".",      TokenType::PUNCTUATION, 8,  3),
        Token("Так",    "так",    TokenType::WORD,        9,  4),
        Token(".",      ".",      TokenType::PUNCTUATION, 10, 4)
    };

    SentenceAnalyzer analyzer;

    assert(analyzer.countTotalPunctuation(text) == 5);

    auto freq = analyzer.analyzePunctuationFrequency(text);
    assert(freq["."] == 2);
    assert(freq[","] == 1);
    assert(freq["!"] == 1);
    assert(freq["?"] == 1);
    assert(freq.count(";") == 0);

    std::cout << "[PASS] testPunctuationAnalysis\n";
}

void testEmptyOrPunctuationOnlyText() {
    // 1. Порожній текст
    Text completelyEmptyText;
    SentenceAnalyzer analyzer;

    assert(analyzer.countSentences(completelyEmptyText) == 0);
    assert(isAlmostEqual(analyzer.calculateAverageSentenceLength(completelyEmptyText), 0.0));
    assert(analyzer.findMaxSentenceLength(completelyEmptyText) == 0);
    assert(isAlmostEqual(analyzer.calculateMedianSentenceLength(completelyEmptyText), 0.0));
    assert(analyzer.countTotalPunctuation(completelyEmptyText) == 0);
    assert(analyzer.analyzePunctuationFrequency(completelyEmptyText).empty());

    // 2. Текст, що складається лише зі знаків пунктуації (речень зі словами немає)
    Text punctuationOnlyText = {
        Token("...", "...", TokenType::PUNCTUATION, 0, 1),
        Token("!",   "!",   TokenType::PUNCTUATION, 1, 2)
    };

    assert(analyzer.countSentences(punctuationOnlyText) == 0);
    assert(isAlmostEqual(analyzer.calculateAverageSentenceLength(punctuationOnlyText), 0.0));
    assert(analyzer.countTotalPunctuation(punctuationOnlyText) == 2);

    std::cout << "[PASS] testEmptyOrPunctuationOnlyText\n";
}

int main() {
    testSentenceMetricsOddCount();
    testMedianEvenCount();
    testPunctuationAnalysis();
    testEmptyOrPunctuationOnlyText();

    std::cout << "All SentenceAnalyzer tests passed!\n";
    return 0;
}