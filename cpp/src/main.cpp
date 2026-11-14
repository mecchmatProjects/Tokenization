#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <iomanip>
#include "../include/MockData.h"
#include "../include/Dataset.h"
#include "../include/Vocabulary.h"
#include "../include/StatisticsAnalyzer.h"
#include "../include/Exporter.h"

namespace fs = std::filesystem;

static void previewCsvFile(const std::string& filePath, size_t maxLines = 6) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        std::cout << "  [Error] Could not open " << filePath << "\n";
        return;
    }

    std::cout << "--- Preview: " << filePath << " ---\n";
    std::string line;
    size_t lineCount = 0;
    while (lineCount < maxLines && std::getline(in, line)) {
        std::cout << "  " << line << "\n";
        ++lineCount;
    }
    std::cout << "\n";
}

int main() {
    Dataset dataset = MockData::createSampleDataset();

    Vocabulary vocab;
    vocab.buildFromDataset(dataset, 1, 0);
    dataset.vectorizeAll(vocab);

    const std::string outputDir = "export_output";
    bool exported = Exporter::exportAllToDirectory(dataset, vocab, outputDir);

    std::cout << "=== 1. Batch CSV Export Status ===\n";
    std::cout << "Export to directory '" << outputDir << "/': "
              << (exported ? "SUCCESS" : "FAILED") << "\n\n";

    const std::vector<std::string> expectedFiles = {
        "tokens.csv",
        "vocabulary.csv",
        "encoded_dataset.csv",
        "bow_matrix.csv",
        "tfidf_matrix.csv",
        "corpus_statistics.csv",
        "token_frequencies.csv",
        "token_types.csv"
    };

    std::cout << std::left << std::setw(26) << "CSV File Name"
              << std::setw(12) << "Status"
              << "Size (bytes)\n";
    std::cout << std::string(52, '-') << "\n";

    for (const auto& fileName : expectedFiles) {
        fs::path fullPath = fs::path(outputDir) / fileName;
        bool exists = fs::exists(fullPath);
        uintmax_t fileSize = exists ? fs::file_size(fullPath) : 0;

        std::cout << std::left << std::setw(26) << fileName
                  << std::setw(12) << (exists && fileSize > 0 ? "OK" : "MISSING")
                  << fileSize << "\n";
    }
    std::cout << "\n";

    std::cout << "=== 2. CSV Content Verification ===\n";
    previewCsvFile(outputDir + "/tokens.csv", 6);
    previewCsvFile(outputDir + "/vocabulary.csv", 8);
    previewCsvFile(outputDir + "/encoded_dataset.csv", 5);
    previewCsvFile(outputDir + "/tfidf_matrix.csv", 5);
    previewCsvFile(outputDir + "/corpus_statistics.csv", 15);

    // Clean up generated test directory after verification
    std::error_code ec;
    fs::remove_all(outputDir, ec);

    return 0;
}