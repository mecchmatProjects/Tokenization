#include <iostream>
#include <clocale> // Add for aetlocale
#include <vector>
#include "text_loader.h"
#include "preprocessor.h"
#include "text_document.h"

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
        std::cout << "--- Document: " << doc.getTitle() << " (" << doc.getYear() << ") ---\n";
        
        // Clean the raw content using our Preprocessor
        // Flags: removePunctuation = true, toLowerCase = true
        std::string cleanedText = Preprocessor::process(doc.getContent(), true, true);
        
        // Print a small preview (first 200 characters) to verify it works
        // We check the length first to avoid errors if the text is very short
        if (cleanedText.length() > 200) {
            std::cout << cleanedText.substr(0, 200) << "...\n\n";
        } else {
            std::cout << cleanedText << "\n\n";
        }
    }
    
    return 0;
}