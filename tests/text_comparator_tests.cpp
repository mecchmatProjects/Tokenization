#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>

#include "style_profile.h"
#include "text_comparator.h"

using namespace litcritic;

void testIdenticalProfiles() {
    StyleProfile p1;
    p1.setTotalWords(100);
    p1.setLexicalStats(100, 60, 55, 0.60, 4.5);
    p1.setSentenceStats(12.0, 3.2);

    StyleProfile p2;
    p2.setTotalWords(100);
    p2.setLexicalStats(100, 60, 55, 0.60, 4.5);
    p2.setSentenceStats(12.0, 3.2);

    StyleComparisonResult res = TextComparator::compare(p1, p2);

    assert(res.wordCountDiff == 0);
    assert(std::abs(res.lexicalDiversityDiff - 0.0) < 1e-6);
    assert(std::abs(res.averageSentenceLengthDiff - 0.0) < 1e-6);
    assert(std::abs(res.normalizedDistance - 0.0) < 1e-6);
    assert(std::abs(res.similarityScore - 1.0) < 1e-6);
}

void testDifferentProfiles() {
    // Короткі динамічні речення, висока лексична різноманітність
    StyleProfile p1;
    p1.setTotalWords(200);
    p1.setLexicalStats(200, 160, 140, 0.80, 4.2);
    p1.setSentenceStats(6.0, 1.5);

    // Довгі складні речення, нижча лексична різноманітність
    StyleProfile p2;
    p2.setTotalWords(500);
    p2.setLexicalStats(500, 150, 130, 0.30, 5.8);
    p2.setSentenceStats(31.0, 6.0);

    StyleComparisonResult res = TextComparator::compare(p1, p2);

    assert(res.wordCountDiff == 300);
    assert(std::abs(res.lexicalDiversityDiff - 0.50) < 1e-6);
    assert(std::abs(res.averageSentenceLengthDiff - 25.0) < 1e-6);
    assert(res.normalizedDistance > 0.0);
    assert(res.similarityScore < 1.0 && res.similarityScore >= 0.0);
}

void testCosineSimilarity() {
    std::map<std::string, int> dict1 = { { "in the", 4 },{ "of the", 2 } };
    std::map<std::string, int> dict2 = { { "in the", 2 },{ "of the", 1 } };
    std::map<std::string, int> dict3 = { { "on a", 3 } };

    // Пропорційні вектори мають косинусну подібність 1.0
    assert(std::abs(TextComparator::calculateCosineSimilarity(dict1, dict2) - 1.0) < 1e-6);
    // Ортогональні вектори (без спільних фраз) мають подібність 0.0
    assert(std::abs(TextComparator::calculateCosineSimilarity(dict1, dict3) - 0.0) < 1e-6);
}

int main() {
    testIdenticalProfiles();
    testDifferentProfiles();
    testCosineSimilarity();
    std::cout << "All TextComparator tests passed successfully!\n";
    return 0;
}