#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
<<<<<<< HEAD
#include "../include/TfIdfEncoder.h"
=======
#include "../include/OneHotEncoder.h"
#include "../include/BagOfWordsEncoder.h"

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i + 1 < vec.size() ? ", " : "");
    }
    std::cout << "]";
}
>>>>>>> origin/develop_cpp

int main() {
    Dataset dataset = MockData::createSampleDataset();

    // 1. Build full vocabulary (minFrequency = 1, unlimited size -> |V| = 22)
    Vocabulary fullVocab;
    fullVocab.buildFromDataset(dataset, 1, 0);

<<<<<<< HEAD
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
=======
    OneHotEncoder oneHotEncoder;
    BagOfWordsEncoder bowEncoder;

    oneHotEncoder.encodeDataset(dataset, fullVocab);
    bowEncoder.encodeDataset(dataset, fullVocab);

    // 2. Inspect 2D Token-Level One-Hot Matrix on Document #3 (5 tokens x 22 vocab size)
    const Document& doc3 = dataset.getDocuments()[2];
    auto doc3Matrix = oneHotEncoder.encodeTokensMatrix(doc3.tokens, fullVocab);

    std::cout << "=== 1. Token-Level 2D One-Hot Matrix (Document #3, "
              << doc3Matrix.size() << "x" << fullVocab.size() << ") ===\n";
    for (size_t i = 0; i < doc3.tokens.size(); ++i) {
        std::cout << std::left << std::setw(12) << doc3.tokens[i].text
                  << " (ID " << std::setw(2) << doc3.tokens[i].tokenId << "): ";
        printVector(doc3Matrix[i]);
        std::cout << "\n";
    }
    std::cout << "Matrix sparsity: " << std::fixed << std::setprecision(2)
              << OneHotEncoder::calculateMatrixSparsity(doc3Matrix) * 100.0 << "% zeros\n\n";

    // 3. Sparsity & Memory Scaling Analysis on Large Vocabularies
    std::cout << "=== 2. One-Hot Sparsity & Memory Analysis (for T = 10,000 tokens) ===\n";
    for (size_t vocabSize : {static_cast<size_t>(22), static_cast<size_t>(1000), static_cast<size_t>(10000), static_cast<size_t>(50000)}) {
        double sparsity = OneHotEncoder::calculateTheoreticalTokenSparsity(vocabSize) * 100.0;
        double memMB = static_cast<double>(OneHotEncoder::estimateMatrixMemoryBytes(10000, vocabSize)) / (1024.0 * 1024.0);
        std::cout << "|V| = " << std::setw(6) << vocabSize
                  << " -> Sparsity: " << std::fixed << std::setprecision(4) << std::setw(8) << sparsity << "%"
                  << " | 2D Matrix Memory: " << std::setprecision(2) << memMB << " MB\n";
    }
    std::cout << "\n";

    // 4. Compare Document-Level Binary One-Hot vs Bag-of-Words on Filtered Vocabulary (minFrequency = 2, |V| = 9)
    Vocabulary filteredVocab;
    filteredVocab.buildFromDataset(dataset, 2, 0);

    oneHotEncoder.encodeDataset(dataset, filteredVocab);
    bowEncoder.encodeDataset(dataset, filteredVocab);

    std::cout << "=== 3. Binary One-Hot vs Bag-of-Words (Filtered Vocab minFrequency = 2, |V| = "
              << filteredVocab.size() << ") ===\n";
    for (const auto& doc : dataset.getDocuments()) {
        std::cout << "Doc #" << doc.id << " (tokens=" << doc.tokens.size() << "):\n";
        std::cout << "  Binary One-Hot : ";
        printVector(doc.oneHotVector);
        std::cout << " (sparsity: " << std::fixed << std::setprecision(2)
                  << OneHotEncoder::calculateVectorSparsity(doc.oneHotVector) * 100.0 << "%)\n";

        std::cout << "  Bag-of-Words   : ";
        printVector(doc.bowVector);
        std::cout << " (total count: " << BagOfWordsEncoder::getTotalTokenCount(doc.bowVector) << ")\n";

        std::cout << "  Non-zero BoW   : ";
        for (const auto& [id, count] : BagOfWordsEncoder::getNonZeroEntries(doc.bowVector)) {
            std::cout << filteredVocab.getTokenText(id) << "(ID " << id << ")=" << count << "  ";
        }
        std::cout << "\n------------------------------------------------------------\n";
    }
>>>>>>> origin/develop_cpp

    return 0;
}