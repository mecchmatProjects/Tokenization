#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"

int main() {
    // 1. Load mock dataset (24 total tokens across 3 documents)
    Dataset dataset = MockData::createSampleDataset();

    // 2. Build bidirectional vocabulary
    Vocabulary vocab;
    vocab.buildFromDataset(dataset);
    vocab.assignIdsToDataset(dataset);

    std::cout << "=== Day 2: Bidirectional Vocabulary Test ===\n";
    std::cout << "Total documents: " << dataset.size() << "\n";
    std::cout << "Unique tokens in vocabulary: " << vocab.size() << "\n\n";

    // 3. Print vocabulary table (ID, Token, Frequency)
    std::cout << std::left << std::setw(8) << "ID" 
              << std::setw(16) << "Token" 
              << "Frequency\n";
    std::cout << "------------------------------------\n";

    for (size_t id = 0; id < vocab.size(); ++id) {
        std::string text = vocab.getTokenText(static_cast<int>(id));
        int freq = vocab.getTokenFrequency(text);
        std::cout << std::left << std::setw(8) << id 
                  << std::setw(16) << text 
                  << freq << "\n";
    }

    // 4. Test bidirectional lookup (Token -> ID and ID -> Token)
    std::cout << "\n=== Lookup Verification ===\n";
    std::string testWord = "machine";
    int id = vocab.getTokenId(testWord);
    std::cout << "Forward lookup ('" << testWord << "' -> ID): " << id << "\n";
    std::cout << "Reverse lookup (ID " << id << " -> Token): '" << vocab.getTokenText(id) << "'\n";
    std::cout << "Unknown word lookup ('neural' -> ID): " << vocab.getTokenId("neural") << "\n";

    // 5. Verify that tokens inside Document #1 received their IDs
    std::cout << "\n=== Document #1 Tokens with Assigned IDs ===\n";
    const auto& doc1 = dataset.getDocuments()[0];
    for (const auto& token : doc1.tokens) {
        std::cout << token.text << "(" << token.tokenId << ") ";
    }
    std::cout << "\n";

    return 0;
}