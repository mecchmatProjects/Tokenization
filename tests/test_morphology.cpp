#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../cpp/include/Word.h"

void testSyllableSplitting() {
    std::cout << "Testing Syllable Splitting & Merging..." << std::endl;

    std::vector<std::string> syllables = Word::splitWordIntoSyllables("вікно");
    assert(syllables.size() == 2 && syllables[0] == "вік" && syllables[1] == "но");
    assert(Word::mergeSyllables(syllables) == "вікно");

    syllables = Word::splitWordIntoSyllables("яблуко");
    assert(syllables.size() == 3 && syllables[0] == "яб" && syllables[1] == "лу" && syllables[2] == "ко");
    assert(Word::mergeSyllables(syllables) == "яблуко");

    syllables = Word::splitWordIntoSyllables("комп'ютер");
    assert(syllables.size() == 3 && syllables[0] == "ком" && syllables[1] == "п'ю" && syllables[2] == "тер");
    assert(Word::mergeSyllables(syllables) == "комп'ютер");

    syllables = Word::splitWordIntoSyllables("молоко");
    assert(syllables.size() == 3 && syllables[0] == "мо" && syllables[1] == "ло" && syllables[2] == "ко");
    assert(Word::mergeSyllables(syllables) == "молоко");

    Word wordObj("вікно");
    assert(wordObj.getHyphenatedWord() == "вік-но");

    std::cout << "Syllable splitting tests passed!\n" << std::endl;
}

void testMorphemicAnalysis() {
    std::cout << "Testing Morphemic Analysis & Inflection..." << std::endl;

    Noun testNoun("приказка", Gender::Feminine);
    assert(testNoun.getType() == PartOfSpeech::Noun);
    assert(testNoun.getPrefixes().size() == 1 && testNoun.getPrefixes()[0] == "при");
    assert(testNoun.getRoots().size() == 1 && testNoun.getRoots()[0] == "каз");
    assert(testNoun.getSuffixes().size() == 1 && testNoun.getSuffixes()[0] == "к");
    assert(testNoun.getEnding() == "а");
    assert(testNoun.getFullWord() == "приказка");

    testNoun.changeNumberAndCase(GrammaticalNumber::Plural, GrammaticalCase::Instrumental);
    assert(testNoun.getFullWord() == "приказками");

    Verb testVerb("прочитати");
    assert(testVerb.getType() == PartOfSpeech::Verb);
    assert(testVerb.getPrefixes().size() == 1 && testVerb.getPrefixes()[0] == "про");
    assert(testVerb.getRoots().size() == 1 && testVerb.getRoots()[0] == "чита");
    assert(testVerb.getSuffixes().empty());
    assert(testVerb.getEnding() == "ти");

    testVerb.changePersonAndNumber(1, GrammaticalNumber::Plural);
    assert(testVerb.getFullWord() == "прочитаємо");

    // Verify pipeline hook used by Application::setMorphologyNormalizer
    assert(Word::normalizeTokenText("приказка") == "каз");

    std::cout << "Morphemic analysis tests passed!\n" << std::endl;
}

int main() {
    std::cout << "--- Running Morphology & Word Tests ---\n" << std::endl;

    testSyllableSplitting();
    testMorphemicAnalysis();

    std::cout << "--- All Morphology Tests Completed Successfully! ---" << std::endl;
    return 0;
}