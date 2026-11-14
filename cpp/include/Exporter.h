#pragma once
#include <string>
#include <vector>
#include "Dataset.h"
#include "Vocabulary.h"
#include "StatisticsAnalyzer.h"

class Exporter {
public:
    Exporter() = default;

    // Escapes a string field according to RFC 4180 CSV rules
    static std::string escapeCsvField(const std::string& field, char delimiter = ',');

    // Exports all extracted tokens across all documents with positions, types, and IDs
    static bool exportTokensToCsv(const Dataset& dataset,
                                  const std::string& filePath,
                                  char delimiter = ',');

    // Exports vocabulary mapping (ID, token text, corpus frequency, special token flag)
    static bool exportVocabularyToCsv(const Vocabulary& vocab,
                                      const std::string& filePath,
                                      char delimiter = ',');

    // Exports document metadata and encoded integer token sequences
    static bool exportEncodedDatasetToCsv(const Dataset& dataset,
                                          const std::string& filePath,
                                          char delimiter = ',');

    // Exports document-by-vocabulary Bag-of-Words count matrix
    static bool exportBowMatrixToCsv(const Dataset& dataset,
                                     const Vocabulary& vocab,
                                     const std::string& filePath,
                                     char delimiter = ',');

    // Exports document-by-vocabulary TF-IDF weight matrix
    static bool exportTfIdfMatrixToCsv(const Dataset& dataset,
                                       const Vocabulary& vocab,
                                       const std::string& filePath,
                                       int precision = 6,
                                       char delimiter = ',');

    // Exports summary corpus statistics as key-value CSV rows
    static bool exportCorpusStatisticsToCsv(const CorpusStatistics& stats,
                                            const std::string& filePath,
                                            char delimiter = ',');

    // Exports sorted token frequency table
    static bool exportTokenFrequenciesToCsv(const std::vector<TokenFrequencyInfo>& table,
                                            const std::string& filePath,
                                            char delimiter = ',');

    // Exports token category distribution table (WORD, NUMBER, PUNCTUATION, SPECIAL, etc.)
    static bool exportTokenTypeDistributionToCsv(const std::vector<TokenTypeStats>& distribution,
                                                 const std::string& filePath,
                                                 char delimiter = ',');

    // Batch-exports all dataset tables, matrices, vocabulary, and analytics into a target directory
    static bool exportAllToDirectory(const Dataset& dataset,
                                     const Vocabulary& vocab,
                                     const std::string& outputDir,
                                     char delimiter = ',');
};