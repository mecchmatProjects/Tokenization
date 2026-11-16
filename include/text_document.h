#ifndef TEXT_DOCUMENT_H
#define TEXT_DOCUMENT_H

#include <string>
#include <vector>
#include "sentence.h"

class TextDocument {
private:
    // Metadata
    std::string title;
    std::string author;

    // Content
    std::vector<Sentence> sentences;

public:
    // Constructor
    TextDocument(const std::string& docTitle, const std::string& docAuthor);

    // Metadata getters
    std::string getTitle() const;
    std::string getAuthor() const;

    // Content managment
    void addSentence(const Sentence& sentence);

    const std::vector<Sentence>& getSentences() const;
};

#endif