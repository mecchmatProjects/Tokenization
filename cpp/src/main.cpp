#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <numeric>
#include "../include/MockData.h"
#include "../include/Dataset.h"
#include "../include/Vocabulary.h"
#include "../include/IntegerEncoder.h"
#include "../include/OneHotEncoder.h"
#include "../include/BagOfWordsEncoder.h"
#include "../include/TfIdfEncoder.h"

namespace fs = std::filesystem;

static void printIntVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i + 1 < vec.size() ? ", " : "");
    }
    std::cout << "]";
}

int main() {
    // 1. Unified Vectorization on Mock Dataset
    std::cout << "=== 1. Unified Dataset Vectorization (Mock Corpus) ===\n";
    Dataset mockDataset = MockData::createSampleDataset();

    Vocabulary vocab;
    vocab.buildFromDataset(mockDataset, 1, 0);

    IntegerEncoder customIntEncoder;
    OneHotEncoder oneHotEncoder;
    BagOfWordsEncoder bowEncoder;
    TfIdfEncoder tfidfEncoder(false);

    mockDataset.vectorizeAll(vocab, customIntEncoder, oneHotEncoder, bowEncoder, tfidfEncoder);

    std::cout << "Vocabulary Size: " << vocab.size() << " tokens\n";
    for (const auto& doc : mockDataset.getDocuments()) {
        int activeOneHot = std::accumulate(doc.oneHotVector.begin(), doc.oneHotVector.end(), 0);
        int totalBow = std::accumulate(doc.bowVector.begin(), doc.bowVector.end(), 0);
        double tfidfSum = std::accumulate(doc.tfidfVector.begin(), doc.tfidfVector.end(), 0.0);

        std::cout << "Doc #" << doc.id << " (" << doc.filePath << "):\n";
        std::cout << "  Integer Seq (len=" << doc.integerSequence.size() << "): ";
        printIntVector(doc.integerSequence);
        std::cout << "\n";
        std::cout << "  One-Hot active indices: " << activeOneHot << " / " << doc.oneHotVector.size() << "\n";
        std::cout << "  BoW total token count : " << totalBow << "\n";
        std::cout << "  TF-IDF weights sum    : " << std::fixed << std::setprecision(4) << tfidfSum << "\n";
    }

    // 2. Directory Batch Loading Test
    std::cout << "\n=== 2. Directory Batch Loading (loadFromDirectory) ===\n";
    const std::string tempDir = "temp_corpus";
    fs::create_directories(tempDir);

    {
        std::ofstream(tempDir + "/doc1.txt") << "Machine learning models process text data.";
        std::ofstream(tempDir + "/doc2.txt") << "Text tokenization builds vocabulary from data.";
        std::ofstream(tempDir + "/doc3.txt") << "Vectorization transforms tokens into numeric vectors.";
        std::ofstream(tempDir + "/doc4.txt") << "Machine learning pipelines use TF-IDF vectors!";
    }

    Dataset dirDataset;
    size_t loadedFiles = dirDataset.loadFromDirectory(tempDir, ".txt", true);
    std::cout << "Loaded " << loadedFiles << " files from directory '" << tempDir << "/'\n";

    Vocabulary dirVocab;
    dirVocab.buildFromDataset(dirDataset, 1, 0);
    dirDataset.vectorizeAll(dirVocab);

    for (const auto& doc : dirDataset.getDocuments()) {
        std::cout << "  Doc #" << doc.id << " [" << doc.filePath << "] -> "
                  << doc.tokens.size() << " tokens, IntSeq=";
        printIntVector(doc.integerSequence);
        std::cout << "\n";
    }

    // 3. Batch Processing (Mini-Batch Splitting & Iteration)
    std::cout << "\n=== 3. Batch Processing (batchSize = 2) ===\n";
    size_t batchSize = 2;
    std::vector<Dataset> batches = dirDataset.splitIntoBatches(batchSize);
    std::cout << "Total Batches Created: " << batches.size() << "\n";

    dirDataset.processInBatches(batchSize, dirVocab, [](Dataset& batch, size_t batchIdx) {
        std::cout << "  Batch #" << (batchIdx + 1)
                  << " (documents=" << batch.size() << "): ";
        for (size_t i = 0; i < batch.size(); ++i) {
            const auto& d = batch.getDocuments()[i];
            std::cout << "Doc#" << d.id << "(tokens=" << d.tokens.size()
                      << ", tfidf_dim=" << d.tfidfVector.size() << ")"
                      << (i + 1 < batch.size() ? ", " : "");
        }
        std::cout << "\n";
    });

    // Clean up temporary directory so git working tree stays clean
    fs::remove_all(tempDir);

    return 0;
}