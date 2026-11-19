#include "text_document.h"

// Constructor: set the title and author of the document
TextDocument::TextDocument(const std::string& docTitle, const std::string& docAuthor, 
                           int docYear, const std::string& docLanguage, const std::string& docContent) {
    title = docTitle;
    author = docAuthor;
    year = docYear;
    language = docLanguage;
    content = docContent;
    // The 'sentences' vector initializes itself automatically as empty
}

// Getters
std::string TextDocument::getTitle() const {
    return title;
}

std::string TextDocument::getAuthor() const {
    return author;
}

int TextDocument::getYear() const {
    return year;
}

std::string TextDocument::getLanguage() const {
    return language;
}

std::string TextDocument::getContent() const {
    return content;
}

// Method to add a fully constructed sentence into the document
void TextDocument::addSentence(const Sentence& sentence) {
    sentences.push_back(sentence);
}

// Getter returning a read-only reference to the list of all aentences
const std::vector<Sentence>& TextDocument::getSentences() const {
    return sentences;
}