#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../cpp/include/Word.h"

void testSyllableSplitting() {
    std::cout << "Testing Syllable Splitting..." << std::endl;

    std::vector<std::string> syllables = Word::splitWordIntoSyllables("вікно");
    assert(syllables.size() == 2 && syllables[0] == "ві" && syllables[1] == "кно");

    syllables = Word::splitWordIntoSyllables("яблуко");
    assert(syllables.size() == 3 && syllables[0] == "я" && syllables[1] == "блу" && syllables[2] == "ко");

    syllables = Word::splitWordIntoSyllables("комп'ютер");
    assert(syllables.size() == 3 && syllables[0] == "ко" && syllables[1] == "мп'ю" && syllables[2] == "тер");

    syllables = Word::splitWordIntoSyllables("відзеленчати"); // digraph dz
    assert(syllables.size() == 5 && syllables[0] == "ві" && syllables[1] == "дзе" && syllables[2] == "ле" && syllables[3] == "нча" && syllables[4] == "ти");

    std::cout << "Syllable splitting tests passed!\n" << std::endl;
}

void testMorphemicAnalysis() {
    std::cout << "Testing Morphemic Analysis..." << std::endl;

    Noun testNoun("приказка");

    // Testing logic for prefix 'при', root 'каз', suffix 'к', ending 'а'
    assert(testNoun.getPrefixes().size() == 1 && testNoun.getPrefixes()[0] == "при");
    assert(testNoun.getRoots().size() == 1 && testNoun.getRoots()[0] == "каз");
    assert(testNoun.getSuffixes().size() == 1 && testNoun.getSuffixes()[0] == "к");
    assert(testNoun.getEnding() == "а");

    Verb testVerb("зробити");

    assert(testVerb.getPrefixes().size() == 1 && testVerb.getPrefixes()[0] == "з");
    assert(testVerb.getRoots().size() == 1 && testVerb.getRoots()[0] == "роб");
    assert(testVerb.getSuffixes().size() == 1 && testVerb.getSuffixes()[0] == "и"); // and 'ти' as infinitive suffix if implemented that way

    std::cout << "Morphemic analysis tests passed!\n" << std::endl;
}

int main() {
    std::cout << "--- Running Morphology & Word Tests ---\n" << std::endl;

    testSyllableSplitting();
    testMorphemicAnalysis();

    std::cout << "--- All Morphology Tests Completed Successfully! ---" << std::endl;
    return 0;
}
