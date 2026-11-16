#include "style_profile.h"

// Constructor: initialize all statistics with zeros
StyleProfile::StyleProfile() {
    totalWords = 0;
    uniqueWords = 0;
    lexicalDiversity = 0.0;
    averageSentenceLength = 0.0;
    maxSentenceLength = 0;
}

// Getters for rading the statistics
int StyleProfile::getTotalWords() const {
    return totalWords;
}

int StyleProfile::getUnigueWords() const {
    return uniqueWords;
}

double StyleProfile::getLexicalDiversity() const {
    return lexicalDiversity;
}

// Setters for updating the stastics (used by analyzers later)
void StyleProfile::setTotalWords(int count) {
    totalWords = count;
}

void StyleProfile::setUniqueWords(int count) {
    uniqueWords = count;
}

void StyleProfile::setLexicalDiversity(double diversity) {
    lexicalDiversity = diversity;
}