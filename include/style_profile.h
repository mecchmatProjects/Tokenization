#ifndef STYLE_PROFILE_H
#define STYLE_PROFILE_H

#include <map>
#include <string>

class StyleProfile {
private:
    // Basic lexical statistics
    int totalWords;
    int uniqueWords;
    int lemmaCount;
    double lexicalDiversity;

    // Sentence statistics
    double averageWordLength;
    double averageSentenceLength;
    int maxSentenceLength;
    double sentenceLengthStdDev;
    double repetitionIndex;

    // Dictionaries for complex statistics
    std::map<std::string, int> punctuationStatistics;
    std::map<std::string, double> partOfSpeechStatistics; // e.g. "NOUN" -> 24.1
    std::map<std::string, int> ngramStatistics;


public:
    // Constructor to initialize everything with zeros
    StyleProfile();

    // Getters (functions to read data)
    int getTotalWords() const;
    double getLexicalDiversity() const;
    double getAverageSentenceLength() const;

    // Setters (functions to write data)
    void setTotalWords(int count);
    void setLexicalStats(int words, int unique, int lemmas, double diversity, double avgWordLen);
    void setSentenceStats(double avgLen, double stDev);
    void setRepetitionIndex(double index);

    void addPunctuationStat(const std::string& mark, int count);
    void addPartOfSpeechStat(const std::string& pos, double percentage);
    void addNgramStat(const std::string& ngram, int count);
};

#endif