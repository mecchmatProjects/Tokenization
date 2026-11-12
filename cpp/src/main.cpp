#include <iostream>
#include <iomanip>
#include "../include/MockData.h"
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

int main() {
    Dataset dataset = MockData::createSampleDataset();

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

    return 0;
}