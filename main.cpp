#include <iostream>
#include <clocale> // Add for aetlocale
#include <vector>
#include <string>

#include "text_loader.h"
#include "preprocessor.h"
#include "text_document.h"
#include "tokenizer.h"


// Helper function to convert TokenType enum to readable text
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::WORD: return "WORD";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::PUNCTUATION: return "PUNCTUATION";
        case TokenType::SPECIAL: return "SPECIAL";
        default: return "UNKNOWN";
    }
}


int main() {
    // Set global locale to Ukrainian UTF-8 to enable correct character classification
    std::setlocale(LC_ALL, "uk_UA.UTF-8");


    std::cout << "Start LitCritic...\n";

    // Load all documents listed in the CSV catalog
    std::string catalogPath = "datasets/catalog.csv";
    std::vector<TextDocument> documents = TextLoader::loadFromCatalog(catalogPath);

    std::cout << "Loaded " << documents.size() << " documents from catalog.\n\n";

    // Iterate through each loaded document
    for (const auto& doc : documents) {
        std::cout << "========================================\n";
        std::cout << "Document: " << doc.getTitle() << " (" << doc.getYear() << ")\n";
        std::cout << "========================================\n";

        // Keep punctuation and original case so Tokenizer can detect sentences and original words
        std::string cleanedText = Preprocessor::process(doc.getContent(), false, false);

        // Tokenize the cleaned text
        std::vector<Token> tokens = Tokenizer::tokenize(cleanedText);
        std::cout << "Total tokens: " << tokens.size() << "\n";

        // Print the first 15 tokens as a preview
        size_t previewLimit = std::min<size_t>(15, tokens.size());
        std::cout << "First " << previewLimit << " tokens:\n";

        for (size_t i = 0; i < previewLimit; ++i) {
            const Token& t = tokens[i];
            std::cout << "  [" << t.getPosition() << "] "
                      << "Sent #" << t.getSentenceNumber() << " | "
                      << tokenTypeToString(t.getType()) << " | "
                      << "text: \"" << t.getText() << "\" | "
                      << "norm: \"" << t.getNormalizedText() << "\"\n";
        }
        std::cout << "\n";
    }

    return 0;
}