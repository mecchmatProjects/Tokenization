#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
#include "../include/IntegerEncoder.h"

void printAlignedDataset(const Dataset& dataset, const Vocabulary& vocab, const std::string& title) {
    std::cout << "=== " << title << " ===\n";
    for (const auto& doc : dataset.getDocuments()) {
        std::cout << "Doc #" << doc.id << " (len=" << doc.integerSequence.size()
                  << ", pads=" << IntegerEncoder::countPaddingTokens(doc.integerSequence) << "): [";
        for (size_t i = 0; i < doc.integerSequence.size(); ++i) {
            std::cout << doc.integerSequence[i] << (i + 1 < doc.integerSequence.size() ? ", " : "");
        }
        std::cout << "]\n";

        auto decoded = IntegerEncoder().decodeSequence(doc.integerSequence, vocab);
        std::cout << "Decoded: ";
        for (const auto& tokenStr : decoded) {
            std::cout << tokenStr << " ";
        }
        std::cout << "\n------------------------------------------------------------\n";
    }

    size_t totalPads = IntegerEncoder::countDatasetPaddingTokens(dataset);
    double padRatio = IntegerEncoder::calculateDatasetPaddingRatio(dataset) * 100.0;
    std::cout << "Total <PAD> tokens in dataset: " << totalPads
              << " (" << std::fixed << std::setprecision(2) << padRatio << "% of memory)\n\n";
}

int main() {
    Dataset dataset = MockData::createSampleDataset();

    Vocabulary vocab;
    vocab.buildFromDataset(dataset, 1, 0);

    // Scenario 1: Padding shorter sequences up to maxLength = 12
    IntegerEncoder padEncoder(false, 12);
    padEncoder.encodeDataset(dataset, vocab);
    printAlignedDataset(dataset, vocab, "Scenario 1: Post-Padding (maxLength = 12)");

    // Scenario 2: Truncating longer sequences and padding shorter ones to maxLength = 6
    IntegerEncoder truncEncoder(false, 6);
    truncEncoder.encodeDataset(dataset, vocab);
    printAlignedDataset(dataset, vocab, "Scenario 2: Post-Truncation & Padding (maxLength = 6)");

    // Scenario 3: Boundary framing (<START>, <END>) + alignment to maxLength = 8
    IntegerEncoder framedEncoder(true, 8);
    framedEncoder.encodeDataset(dataset, vocab);
    printAlignedDataset(dataset, vocab, "Scenario 3: Boundary Framing + Alignment (maxLength = 8)");

    return 0;
}