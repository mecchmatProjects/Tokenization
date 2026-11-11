#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <numeric>
#include "../include/MockData.h"
#include "../include/Dataset.h"
#include "../include/Vocabulary.h"
#include "../include/StatisticsAnalyzer.h"

void printStatisticsReport(const std::string& title, const CorpusStatistics& stats) {
    std::cout << "=== " << title << " ===\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Documents count:                " << stats.documentCount << "\n";
    std::cout << "Total raw characters:           " << stats.totalRawCharacters << "\n";
    std::cout << "Total token characters:         " << stats.totalTokenCharacters << "\n";
    std::cout << "Total tokens (T):               " << stats.totalTokens << "\n";
    std::cout << "Unique corpus tokens (V_raw):   " << stats.uniqueTokens << "\n";
    std::cout << "In-vocabulary unique tokens:    " << stats.inVocabularyUniqueTokens << "\n";
    std::cout << "Unknown token occurrences:      " << stats.unknownTokens
              << " (" << std::setprecision(2) << (stats.unknownTokenRatio * 100.0) << "%)\n";
    std::cout << "Unique unknown tokens:          " << stats.uniqueUnknownTokens << "\n";
    std::cout << std::setprecision(4);
    std::cout << "Avg document length (chars):    " << stats.averageDocumentLengthChars << "\n";
    std::cout << "Avg document length (tokens):   " << stats.averageDocumentLengthTokens << "\n";
    std::cout << "Avg token length (all):         " << stats.averageTokenLength << "\n";
    std::cout << "Avg unique token length:        " << stats.averageUniqueTokenLength << "\n";
    std::cout << "Avg unknown token length:       " << stats.averageUnknownTokenLength << "\n\n";
}

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

    // 1. Statistics with full unfiltered vocabulary (minFrequency = 1, unlimited)
    Vocabulary fullVocab;
    fullVocab.buildFromDataset(dataset, 1, 0);
    fullVocab.assignIdsToDataset(dataset);
    CorpusStatistics fullStats = StatisticsAnalyzer::analyze(dataset, &fullVocab);
    printStatisticsReport("1. Corpus Statistics (Full Vocabulary: minFrequency = 1)", fullStats);

    // 2. Statistics with frequency-filtered vocabulary (minFrequency = 2)
    Vocabulary filteredVocab;
    filteredVocab.buildFromDataset(dataset, 2, 0);
    filteredVocab.assignIdsToDataset(dataset);
    CorpusStatistics filteredStats = StatisticsAnalyzer::analyze(dataset, &filteredVocab);
    printStatisticsReport("2. Corpus Statistics (Filtered Vocabulary: minFrequency = 2)", filteredStats);

    // 3. Statistics with size-capped vocabulary (maxVocabularySize = 7)
    Vocabulary cappedVocab;
    cappedVocab.buildFromDataset(dataset, 1, 7);
    cappedVocab.assignIdsToDataset(dataset);
    CorpusStatistics cappedStats = StatisticsAnalyzer::analyze(dataset, &cappedVocab);
    printStatisticsReport("3. Corpus Statistics (Capped Vocabulary: maxVocabularySize = 7)", cappedStats);

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