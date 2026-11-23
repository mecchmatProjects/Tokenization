#include "style_profile.h"

// Constructor: initialize all statistics with zeros
StyleProfile::StyleProfile() {
    totalWords = 0;
    uniqueWords = 0;
    lemmaCount = 0;
    lexicalDiversity = 0.0;
    averageWordLength = 0.0;
    averageSentenceLength = 0.0;
    maxSentenceLength = 0;
    sentenceLengthStdDev = 0.0;
    repetitionIndex = 0.0;
}

// Getters for reading the statistics
int StyleProfile::getTotalWords() const {
    return totalWords;
}

int StyleProfile::getUniqueWords() const {
    return uniqueWords;
}

int StyleProfile::getLemmaCount() const {
    return lemmaCount;
}

double StyleProfile::getLexicalDiversity() const {
    return lexicalDiversity;
}

double StyleProfile::getAverageWordLength() const {
    return averageWordLength;
}

double StyleProfile::getAverageSentenceLength() const {
    return averageSentenceLength;
}

int StyleProfile::getMaxSentenceLength() const {
    return maxSentenceLength;
}

double StyleProfile::getSentenceLengthStdDev() const {
    return sentenceLengthStdDev;
}

double StyleProfile::getRepetitionIndex() const {
    return repetitionIndex;
}

const std::map<std::string, int>& StyleProfile::getPunctuationStatistics() const {
    return punctuationStatistics;
}

const std::map<std::string, double>& StyleProfile::getPartOfSpeechStatistics() const {
    return partOfSpeechStatistics;
}

const std::map<std::string, int>& StyleProfile::getNgramStatistics() const {
    return ngramStatistics;
}

// Setters for updating the statistics (used by analyzers later)
void StyleProfile::setTotalWords(int count) {
    totalWords = count;
}

void StyleProfile::setLexicalStats(int words, int unique, int lemmas, double diversity, double avgWordLen) {
    totalWords = words;
    uniqueWords = unique;
    lemmaCount = lemmas;
    lexicalDiversity = diversity;
    averageWordLength = avgWordLen;
}

void StyleProfile::setSentenceStats(double avgLen, double stdDev) {
    averageSentenceLength = avgLen;
    sentenceLengthStdDev = stdDev;
}

void StyleProfile::setMaxSentenceLength(int maxLen) {
    maxSentenceLength = maxLen;
}

void StyleProfile::setRepetitionIndex(double index) {
    repetitionIndex = index;
}

// Inserting data into dictionaries
void StyleProfile::addPunctuationStat(const std::string& mark, int count) {
    punctuationStatistics[mark] = count;
}

void StyleProfile::addPartOfSpeechStat(const std::string& pos, double percentage) {
    partOfSpeechStatistics[pos] = percentage;
}

void StyleProfile::addNgramStat(const std::string& ngram, int count) {
    ngramStatistics[ngram] = count;
}