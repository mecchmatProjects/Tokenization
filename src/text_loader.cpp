#include "text_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

// Loads a single text file and constructs a TextDocument object
TextDocument TextLoader::loadDocument(
    const std::string& filePath,
    const std::string& title,
    const std::string& author,
    int year,
    const std::string& language
) {
    std::ifstream file(filePath);
    std::string content = "";

    // Check if the file was opened successfully
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << "\n";
        // Return an empty document if file if missing to prevent crashes
        return TextDocument(title, author, year, language, content);
    }

    // Read the entire file content into a string stream buffer efficiently
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();

    file.close();

    // Return the constructed document with metadata and content
    return TextDocument(title, author, year, language, content);
}

// Parses a CSV catalog file and loads all documents listed inside
std::vector<TextDocument> TextLoader::loadFromCatalog(const std::string& catalogPath) {
    std::vector<TextDocument> documents;
    std::ifstream catalog(catalogPath);

    // Check if the catalog file exists
    if (!catalog.is_open()) {
        std::cerr << "Error: Could not open catalog " << catalogPath << "\n";
        return documents;
    }

    std::string line;
    // Read the file line by line
    while (std::getline(catalog, line)) {
        // Skip empty lines to avoid parsing errors
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string relativePath, title, author, yearStr, language;

        // Parse fields separated by ';' based on the CSV format
        std::getline(ss, relativePath, ';');
        std::getline(ss, title, ';');
        std::getline(ss, author, ';');
        std::getline(ss, yearStr, ';');
        std::getline(ss, language, ';');

        // Skip malformed rows where essentuial fields are missing
        if (relativePath.empty() || title.empty() || author.empty()) {
            std::cerr << "Warning: Skipping malformed row in catalog\n";
            continue;
        }

        // Construct the full path by prepending "datasets/" directory
        std::string fullPath = "datasets/" + relativePath;

        // Convert year from string to integer safely
        int year = 0;
        try {
            year = std::stoi(yearStr);
        } catch (...) {
            std::cerr << "Warning: Invalid year format in catalog for " << title << "\n";
        }

        // Load the document using our single-file method and add it to the vector
        TextDocument doc = loadDocument(fullPath, title, author, year, language);
        // Only add non-empty documents to prevent breaking analyzers later
        if (doc.getContent().length() > 0) {
            documents.push_back(doc);
        } else {
            std::cerr << "Warning: Document " << title << " is empty or missing, skipping\n";
        }
    }

    catalog.close();
    return documents;
}