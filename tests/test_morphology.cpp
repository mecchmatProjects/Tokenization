#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../cpp/include/Word.h"

void testSyllableSplitting() {
    std::cout << "Testing Syllable Splitting..." << std::endl;

    std::vector<std::string> syllables = Word::splitIntoSyllables("вікно");
    assert(syllables.size() == 2 && syllables[0] == "ві" && syllables[1] == "кно");

    syllables = Word::splitIntoSyllables("яблуко");
    assert(syllables.size() == 3 && syllables[0] == "я" && syllables[1] == "блу" && syllables[2] == "ко");

    syllables = Word::splitIntoSyllables("комп'ютер");
    assert(syllables.size() == 3 && syllables[0] == "ко" && syllables[1] == "мп'ю" && syllables[2] == "тер");

    syllables = Word::splitIntoSyllables("відзеленчати"); // digraph dz
    assert(syllables.size() == 5 && syllables[0] == "ві" && syllables[1] == "дзе" && syllables[2] == "ле" && syllables[3] == "нча" && syllables[4] == "ти");

    std::cout << "Syllable splitting tests passed!\n" << std::endl;
}

void testMorphemicAnalysis() {
    std::cout << "Testing Morphemic Analysis..." << std::endl;

    Noun testNoun("приказка");
    MorphemicStructure structure = testNoun.getMorphemicStructure();

    // Testing logic for prefix 'при', root 'каз', suffix 'к', ending 'а'
    assert(structure.prefixes.size() == 1 && structure.prefixes[0] == "при");
    assert(structure.roots.size() == 1 && structure.roots[0] == "каз");
    assert(structure.suffixes.size() == 1 && structure.suffixes[0] == "к");
    assert(structure.ending == "а");

    Verb testVerb("зробити");
    structure = testVerb.getMorphemicStructure();

    assert(structure.prefixes.size() == 1 && structure.prefixes[0] == "з");
    assert(structure.roots.size() == 1 && structure.roots[0] == "роб");
    assert(structure.suffixes.size() == 1 && structure.suffixes[0] == "и"); // and 'ти' as infinitive suffix if implemented that way

    std::cout << "Morphemic analysis tests passed!\n" << std::endl;
}

int main() {
    std::cout << "--- Running Morphology & Word Tests ---\n" << std::endl;

    testSyllableSplitting();
    testMorphemicAnalysis();

    std::cout << "--- All Morphology Tests Completed Successfully! ---" << std::endl;
    return 0;
} 