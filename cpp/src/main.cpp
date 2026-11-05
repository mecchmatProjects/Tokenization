#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"

int main() {
    // 1. Load mock dataset
    Dataset dataset = MockData::createSampleDataset();

    // 2. Build bidirectional vocabulary with reserved special tokens
    Vocabulary vocab;
    vocab.buildFromDataset(dataset);
    vocab.assignIdsToDataset(dataset);

    std::cout << "=== Bidirectional Vocabulary & Special Tokens Test ===\n";
    std::cout << "Total documents: " << dataset.size() << "\n";
    std::cout << "Total vocabulary size (with special tokens): " << vocab.size() << "\n\n";

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

    // 4. Verify special tokens and unknown word fallback
    std::cout << "\n=== Special Tokens & Lookup Verification ===\n";
    std::cout << "<PAD> ID:   " << vocab.getTokenId(Vocabulary::PAD_TOKEN) << "\n";
    std::cout << "<UNK> ID:   " << vocab.getTokenId(Vocabulary::UNK_TOKEN) << "\n";
    std::cout << "<START> ID: " << vocab.getTokenId(Vocabulary::START_TOKEN) << "\n";
    std::cout << "<END> ID:   " << vocab.getTokenId(Vocabulary::END_TOKEN) << "\n";

    std::string knownWord = "machine";
    int knownId = vocab.getTokenId(knownWord);
    std::cout << "Known word ('" << knownWord << "' -> ID): " << knownId 
              << " -> '" << vocab.getTokenText(knownId) << "'\n";

    std::string unknownWord = "neural";
    int unkId = vocab.getTokenId(unknownWord);
    std::cout << "Out-of-vocabulary word ('" << unknownWord << "' -> ID): " << unkId 
              << " (" << vocab.getTokenText(unkId) << ")\n";

    // 5. Verify Document #1 sequence framed with <START> and <END>
    std::cout << "\n=== Document #1 Sequence with Boundary Markers ===\n";
    std::cout << Vocabulary::START_TOKEN << "(" << Vocabulary::START_ID << ") ";
    for (const auto& token : dataset.getDocuments()[0].tokens) {
        std::cout << token.text << "(" << token.tokenId << ") ";
    }
    std::cout << Vocabulary::END_TOKEN << "(" << Vocabulary::END_ID << ")\n";

    return 0;
}