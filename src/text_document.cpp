#include "text_document.h"

// Constructor: set the title and author of the document
TextDocument::TextDocument(const std::string& docTitle, const std::string& docAuthor) {
    title = docTitle;
    author = docAuthor;
    // The 'sentences' vector initializes itself automatically as empty
}

// Getters
std::string TextDocument::getTitle() const {
    return title;
}

std::string TextDocument::getAuthor() const {
    return author;
}

// Method to add a fully constructed sentence into the document
void TextDocument::addSentence(const Sentence& sentence) {
    sentences.push_back(sentence);
}

// Getter returning a read-only reference to the list of all aentences
const std::vector<Sentence>& TextDocument::getSentences() const {
    return sentences;
}