#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"

void printVocabulary(const Vocabulary& vocab, const std::string& title) {
    std::cout << "=== " << title << " ===\n";
    std::cout << "Total vocabulary size: " << vocab.size() << "\n";
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
    std::cout << "\n";
}

int main() {
    Dataset dataset = MockData::createSampleDataset();

    // 1. Full vocabulary sorted by frequency (minFrequency = 1, unlimited size)
    Vocabulary fullVocab;
    fullVocab.buildFromDataset(dataset, 1, 0);
    printVocabulary(fullVocab, "Sorted Vocabulary (minFrequency = 1, maxVocabularySize = 0)");

    // 2. Filtered vocabulary by minimum frequency (minFrequency = 2)
    Vocabulary freqFilteredVocab;
    freqFilteredVocab.buildFromDataset(dataset, 2, 0);
    freqFilteredVocab.assignIdsToDataset(dataset);
    printVocabulary(freqFilteredVocab, "Filtered Vocabulary (minFrequency = 2)");

    std::cout << "=== Document #1 Tokens After minFrequency = 2 Filtering ===\n";
    for (const auto& token : dataset.getDocuments()[0].tokens) {
        std::string mappedText = freqFilteredVocab.getTokenText(token.tokenId);
        std::cout << token.text << " -> ID " << token.tokenId << " (" << mappedText << ")\n";
    }
    std::cout << "\n";

    // 3. Capped vocabulary by maxVocabularySize = 7 (4 special + top 3 corpus tokens)
    Vocabulary sizeCappedVocab;
    sizeCappedVocab.buildFromDataset(dataset, 1, 7);
    printVocabulary(sizeCappedVocab, "Capped Vocabulary (minFrequency = 1, maxVocabularySize = 7)");

    return 0;
}