#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
#include "../include/IntegerEncoder.h"

void printSequence(const std::vector<int>& seq) {
    std::cout << "[";
    for (size_t i = 0; i < seq.size(); ++i) {
        std::cout << seq[i] << (i + 1 < seq.size() ? ", " : "");
    }
    std::cout << "]";
}

void printDecoded(const std::vector<std::string>& tokens) {
    std::cout << "[";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i] << (i + 1 < tokens.size() ? " " : "");
    }
    std::cout << "]";
}

int main() {
    Dataset dataset = MockData::createSampleDataset();

    // 1. Build full sorted vocabulary
    Vocabulary fullVocab;
    fullVocab.buildFromDataset(dataset, 1, 0);

    // 2. Encode dataset into integer sequences (without boundary tokens)
    IntegerEncoder encoder(false);
    encoder.encodeDataset(dataset, fullVocab);

    std::cout << "=== Standard Integer Encoding (" << encoder.getName() << ") ===\n";
    for (const auto& doc : dataset.getDocuments()) {
        std::cout << "Document #" << doc.id << " (" << doc.tokens.size() << " tokens):\n";
        std::cout << "  Raw text : " << doc.rawText << "\n";
        std::cout << "  IDs      : ";
        printSequence(doc.integerSequence);
        std::cout << "\n  Decoded  : ";
        printDecoded(encoder.decodeSequence(doc.integerSequence, fullVocab));
        std::cout << "\n\n";
    }

    // 3. Encode with <START> (ID 2) and <END> (ID 3) boundary markers
    encoder.setAddBoundaryTokens(true);
    encoder.encodeDataset(dataset, fullVocab);

    std::cout << "=== Integer Encoding with <START> and <END> Boundaries ===\n";
    const auto& doc1 = dataset.getDocuments()[0];
    std::cout << "Document #1 IDs     : ";
    printSequence(doc1.integerSequence);
    std::cout << "\nDocument #1 Decoded : ";
    printDecoded(encoder.decodeSequence(doc1.integerSequence, fullVocab));
    std::cout << "\n\n";

    // 4. Encode with filtered vocabulary (minFrequency = 2) to verify <UNK> (ID 1) mapping
    Vocabulary filteredVocab;
    filteredVocab.buildFromDataset(dataset, 2, 0);

    encoder.setAddBoundaryTokens(false);
    encoder.encodeDataset(dataset, filteredVocab);

    std::cout << "=== Integer Encoding with Filtered Vocabulary (minFrequency = 2) ===\n";
    for (const auto& doc : dataset.getDocuments()) {
        std::cout << "Document #" << doc.id << " IDs     : ";
        printSequence(doc.integerSequence);
        std::cout << "\nDocument #" << doc.id << " Decoded : ";
        printDecoded(encoder.decodeSequence(doc.integerSequence, filteredVocab));
        std::cout << "\n\n";
    }

    return 0;
}