#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
#include "../include/TfIdfEncoder.h"

int main() {
    Dataset dataset = MockData::createSampleDataset();

    // 1. Build full vocabulary (minFrequency = 1, unlimited size -> |V| = 22)
    Vocabulary fullVocab;
    fullVocab.buildFromDataset(dataset, 1, 0);

    TfIdfEncoder tfidfEncoder(false); // Standard formula: IDF(t) = ln(N / df(t))
    tfidfEncoder.encodeDataset(dataset, fullVocab);

    const auto& df = tfidfEncoder.getDocumentFrequencies();
    const auto& idf = tfidfEncoder.getIdfVector();

    std::cout << "=== 1. Corpus Document Frequency (df) & IDF Table (N = "
              << tfidfEncoder.getTotalDocuments() << ", |V| = " << fullVocab.size() << ") ===\n";
    std::cout << std::left << std::setw(6) << "ID"
              << std::setw(14) << "Token"
              << std::setw(11) << "CorpusFreq"
              << std::setw(8) << "df(t)"
              << "IDF(t) = ln(N/df)\n";
    std::cout << "--------------------------------------------------------\n";

    for (size_t id = 0; id < fullVocab.size(); ++id) {
        std::string text = fullVocab.getTokenText(static_cast<int>(id));
        int corpusFreq = fullVocab.getTokenFrequency(text);
        std::cout << std::left << std::setw(6) << id
                  << std::setw(14) << text
                  << std::setw(11) << corpusFreq
                  << std::setw(8) << df[id]
                  << std::fixed << std::setprecision(4) << idf[id] << "\n";
    }
    std::cout << "\n";

    // 2. Detailed TF, IDF, and TF-IDF Breakdown per Document
    std::cout << "=== 2. Document TF-IDF Vectors Breakdown (Full Vocabulary) ===\n";
    for (const auto& doc : dataset.getDocuments()) {
        auto tf = TfIdfEncoder::computeTermFrequency(doc.tokens, fullVocab);
        std::cout << "Document #" << doc.id << " (\"" << doc.rawText << "\", |d| = " << doc.tokens.size() << "):\n";
        std::cout << "  " << std::left << std::setw(6) << "ID"
                  << std::setw(14) << "Token"
                  << std::setw(10) << "TF(t,d)"
                  << std::setw(10) << "IDF(t)"
                  << "TF-IDF(t,d)\n";

        for (size_t id = 0; id < fullVocab.size(); ++id) {
            if (tf[id] > 0.0) {
                std::cout << "  " << std::left << std::setw(6) << id
                          << std::setw(14) << fullVocab.getTokenText(static_cast<int>(id))
                          << std::fixed << std::setprecision(4)
                          << std::setw(10) << tf[id]
                          << std::setw(10) << idf[id]
                          << doc.tfidfVector[id] << "\n";
            }
        }

        std::cout << "  Non-zero TF-IDF entries: ";
        for (const auto& [id, val] : TfIdfEncoder::getNonZeroEntries(doc.tfidfVector)) {
            std::cout << fullVocab.getTokenText(id) << "(ID " << id << ")="
                      << std::fixed << std::setprecision(4) << val << "  ";
        }
        std::cout << "\n------------------------------------------------------------\n";
    }

    // 3. Test TF-IDF with Filtered Vocabulary (minFrequency = 2 -> |V| = 9)
    Vocabulary filteredVocab;
    filteredVocab.buildFromDataset(dataset, 2, 0);
    tfidfEncoder.encodeDataset(dataset, filteredVocab);

    std::cout << "\n=== 3. TF-IDF on Filtered Vocabulary (minFrequency = 2, |V| = "
              << filteredVocab.size() << ") ===\n";
    for (const auto& doc : dataset.getDocuments()) {
        std::cout << "Doc #" << doc.id << " [";
        for (size_t i = 0; i < doc.tfidfVector.size(); ++i) {
            std::cout << std::fixed << std::setprecision(4) << doc.tfidfVector[i]
                      << (i + 1 < doc.tfidfVector.size() ? ", " : "");
        }
        std::cout << "]\n";
    }

    return 0;
}