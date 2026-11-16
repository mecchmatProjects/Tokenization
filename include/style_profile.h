#ifndef STYLE_PROFILE_H
#define STYLE_PROFILE_H

class StyleProfile {
private:
    // Basic lexical statistics
    int totalWords;
    int uniqueWords;
    double lexicalDiversity;

    // Sentence statistics
    double averageSentenceLength;
    int maxSentenceLength;

public:
    // Constructor to initialize everything with zeros
    StyleProfile();

    // Getters (functions to read data)
    int getTotalWords() const;
    int getUnigueWords() const;
    double getLexicalDiversity() const;

    // Setters (functions to write data)
    void setTotalWords(int count);
    void setUniqueWords(int count);
    void setLexicalDiversity(double diversity);

    // May be add more setters/getters for sentense here later
};

#endif