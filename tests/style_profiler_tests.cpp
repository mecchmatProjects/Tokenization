#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "sentence.h"
#include "style_profile.h"
#include "style_profiler.h"
#include "text_document.h"
#include "token.h"

using namespace litcritic;

void testFullProfileAggregation() {
    Sentence s1(1, "The old house stood silent.");
    s1.addToken({ "The", "The", TokenType::WORD, 0, 1 });
    s1.addToken({ "old", "old", TokenType::WORD, 1, 1 });
    s1.addToken({ "house", "house", TokenType::WORD, 2, 1 });
    s1.addToken({ "stood", "stood", TokenType::WORD, 3, 1 });
    s1.addToken({ "silent", "silent", TokenType::WORD, 4, 1 });
    s1.addToken({ ".", ".", TokenType::PUNCTUATION, 5, 1 });

    Sentence s2(2, "The old house was cold.");
    s2.addToken({ "The", "The", TokenType::WORD, 0, 2 });
    s2.addToken({ "old", "old", TokenType::WORD, 1, 2 });
    s2.addToken({ "house", "house", TokenType::WORD, 2, 2 });
    s2.addToken({ "was", "was", TokenType::WORD, 3, 2 });
    s2.addToken({ "cold", "cold", TokenType::WORD, 4, 2 });
    s2.addToken({ ".", ".", TokenType::PUNCTUATION, 5, 2 });

    ::TextDocument textDocument("", "", 0, "", "");
    textDocument.addSentence(s1);
    textDocument.addSentence(s2);

    ProfilerConfig config;
    config.topNGramsLimit = 5;
    config.minPhraseRepetitions = 2;

    StyleProfile profile = StyleProfiler::buildProfile(textDocument, config);

    // Перевірка показників через доступні геттери StyleProfile
    assert(profile.getTotalWords() == 10);
    assert(std::abs(profile.getLexicalDiversity() - 0.7) < 1e-6); // 7 унікальних з 10 слів
    assert(std::abs(profile.getAverageSentenceLength() - 5.0) < 1e-6); // (5 + 5) / 2 = 5.0
}

void testEmptyDocumentProfile() {
    ::TextDocument emptyDoc("", "", 0, "", "");
    StyleProfile profile = StyleProfiler::buildProfile(emptyDoc);

    assert(profile.getTotalWords() == 0);
    assert(std::abs(profile.getLexicalDiversity() - 0.0) < 1e-6);
    assert(std::abs(profile.getAverageSentenceLength() - 0.0) < 1e-6);
}

int main() {
    testFullProfileAggregation();
    testEmptyDocumentProfile();
    std::cout << "All StyleProfiler tests passed successfully!\n";
    return 0;
}