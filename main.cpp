#include <algorithm>
#include <clocale>
#include <iostream>
#include <string>
#include <vector>

#include "lemmatizer.h"
#include "preprocessor.h"
#include "text_document.h"
#include "text_loader.h"
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

// Count total WORD tokens in a token vector
size_t countWords(const std::vector<Token>& tokens) {
    size_t count = 0;
    for (const auto& t : tokens) {
        if (t.getType() == TokenType::WORD) {
            ++count;
        }
    }
    return count;
}

// Run the full pipeline on a single document and print summary + preview
void processAndPrintDocument(
    const TextDocument& doc,
    const Lemmatizer& ukLemmatizer,
    const Lemmatizer& enLemmatizer
) {
    std::cout << "========================================\n";
    std::cout << "Document: " << doc.getTitle() << " (" << doc.getYear()
              << ") [" << doc.getLanguage() << "]\n";
    std::cout << "========================================\n";

    // Keep punctuation and original case so Tokenizer can detect sentences and original words
    std::string cleanedText = Preprocessor::process(doc.getContent(), false, false);
    std::vector<Token> tokens = Tokenizer::tokenize(cleanedText);

    // Handle empty documents gracefully
    if (tokens.empty()) {
        std::cout << "Document is empty (0 tokens, 0 words, 0 sentences).\n\n";
        return;
    }

    // Select the appropriate lemmatizer based on the document language
    std::string lang = Preprocessor::convertToLowerCase(doc.getLanguage());
    if (lang == "en" || lang == "english") {
        enLemmatizer.lemmatizeTokens(tokens);
    } else {
        ukLemmatizer.lemmatizeTokens(tokens);
    }

    size_t wordCount = countWords(tokens);
    int sentenceCount = tokens.back().getSentenceNumber();

    std::cout << "Total tokens: " << tokens.size()
              << " | Words: " << wordCount
              << " | Sentences: " << sentenceCount << "\n";

    // Print the first 15 tokens as a preview
    size_t previewLimit = std::min<size_t>(15, tokens.size());
    std::cout << "First " << previewLimit << " tokens:\n";

    for (size_t i = 0; i < previewLimit; ++i) {
        const Token& t = tokens[i];
        std::cout << "  [" << t.getPosition() << "] "
                  << "Sent #" << t.getSentenceNumber() << " | "
                  << tokenTypeToString(t.getType()) << " | "
                  << "text: \"" << t.getText() << "\" | "
                  << "norm: \"" << t.getNormalizedText() << "\" | "
                  << "lemma: \"" << t.getLemma() << "\"\n";
    }
    std::cout << "\n";
}

int main() {
    // Set global locale to Ukrainian UTF-8 to enable correct character classification
    std::setlocale(LC_ALL, "uk_UA.UTF-8");

    std::cout << "Start LitCritic...\n";

    // Load dictionaries once before processing documents
    Lemmatizer ukLemmatizer;
    if (ukLemmatizer.loadDictionary("datasets/dictionaries/uk_dict.txt")) {
        std::cout << "Loaded UK dictionary: " << ukLemmatizer.getDictionarySize() << " entries.\n";
    } else {
        std::cout << "Warning: Could not load UK dictionary!\n";
    }

    Lemmatizer enLemmatizer;
    if (enLemmatizer.loadDictionary("datasets/dictionaries/en_dict.txt")) {
        std::cout << "Loaded EN dictionary: " << enLemmatizer.getDictionarySize() << " entries.\n";
    } else {
        std::cout << "Warning: Could not load EN dictionary!\n";
    }

    // Load all documents listed in the CSV catalog
    const std::string catalogPath = "datasets/catalog.csv";
    std::vector<TextDocument> documents = TextLoader::loadFromCatalog(catalogPath);

    std::cout << "\nLoaded " << documents.size() << " documents from catalog.\n\n";

    // Process every document from the catalog
    for (const auto& doc : documents) {
        processAndPrintDocument(doc, ukLemmatizer, enLemmatizer);
    }
}