#ifndef TEXT_LOADER_H
#define TEXT_LOADER_H

#include <string>
#include <vector>
#include "text_document.h"

class TextLoader {
public:
    // Loads a single text file and constructs a Text Document object
    static TextDocument loadDocument(
        const std::string& filePath,
        const std::string& title,
        const std::string& author,
        int year,
        const std::string& language
    );

    // Parses a CSV catalog file and loads all documents listed inside
    static std::vector<TextDocument> loadFromCatalog(const std::string& catalogPath);
};

#endif