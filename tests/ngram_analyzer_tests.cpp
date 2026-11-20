#include <cassert>
#include <iostream>
#include <vector>

#include "ngram_analyzer.h"
#include "sentence.h"
#include "token.h"

using namespace litcritic;

void testWordFrequency() {
    std::vector<Token> tokens = {
     { "The", "the", TokenType::WORD, 0, 1 },
        { "cat", "cat", TokenType::WORD, 1, 1 },
        { "and", "and", TokenType::WORD, 2, 1 },
        { "the", "the", TokenType::WORD, 3, 1 },
        { "dog", "dog", TokenType::WORD, 4, 1 },
        { ".",   ".",   TokenType::PUNCTUATION, 5, 1 }
    };


    auto freq = NGramAnalyzer::countWordFrequencies(tokens);
    assert(freq["the"] == 2);
    assert(freq["cat"] == 1);
    assert(freq["dog"] == 1);
    assert(freq.find(".") == freq.end());
}

void testBigramsAndTrigrams() {
    // Речення 1: "in the dark night"
    Sentence s1(1, "In the dark night.");
    s1.addToken({ "In", "in", TokenType::WORD, 0, 1 });
    s1.addToken({ "the", "the", TokenType::WORD, 1, 1 });
    s1.addToken({ "dark", "dark", TokenType::WORD, 2, 1 });
    s1.addToken({ "night", "night", TokenType::WORD, 3, 1 });
    s1.addToken({ ".", ".", TokenType::PUNCTUATION, 4, 1 });

    // Речення 2: "in the dark forest"
    Sentence s2(2, "In the dark forest.");
    s2.addToken({ "In", "in", TokenType::WORD, 0, 1 });
    s2.addToken({ "the", "the", TokenType::WORD, 1, 1 });
    s2.addToken({ "dark", "dark", TokenType::WORD, 2, 1 });
    s2.addToken({ "forest", "forest", TokenType::WORD, 3, 1 });
    s2.addToken({ ".", ".", TokenType::PUNCTUATION, 4, 1 });

    std::vector<Sentence> sentences = { s1, s2 };

    auto bigrams = NGramAnalyzer::extractBigrams(sentences);
    assert(bigrams["in the"] == 2);
    assert(bigrams["the dark"] == 2);
    assert(bigrams["dark night"] == 1);
    // Перевірка, що кінець 1-го речення не склеївся з початком 2-го
    assert(bigrams.find("night in") == bigrams.end());

    auto trigrams = NGramAnalyzer::extractTrigrams(sentences);
    assert(trigrams["in the dark"] == 2);

    auto repeated = NGramAnalyzer::getRepeatedPhrases(trigrams, 2);
    assert(repeated.size() == 1);
    assert(repeated[0].first == "in the dark");
    assert(repeated[0].second == 2);
}

int main() {
    testWordFrequency();
    testBigramsAndTrigrams();
    std::cout << "All NGramAnalyzer tests passed successfully!\n";
    return 0;
}