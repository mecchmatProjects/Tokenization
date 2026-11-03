#include <iostream>
#include "../include/MockData.h"

int main() {
    // Generate mock dataset for independent module testing
    Dataset testDataset = MockData::createSampleDataset();

    std::cout << "=== Mock Dataset Successfully Loaded ===\n";
    std::cout << "Total documents: " << testDataset.size() << "\n\n";

    for (const auto& doc : testDataset.getDocuments()) {
        std::cout << "Document #" << doc.id << " (" << doc.filePath << "):\n";
        std::cout << "Raw text: " << doc.rawText << "\n";
        std::cout << "Tokens (" << doc.tokens.size() << " total): ";
        
        for (const auto& token : doc.tokens) {
            std::cout << "[" << token.text << " : " << tokenTypeToString(token.type) << "] ";
        }
        std::cout << "\n--------------------------------------------\n";
    }

    return 0;
}