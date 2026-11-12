#include "../include/Exporter.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
    void ensureParentDirectory(const std::string& filePath) {
        fs::path pathObj(filePath);
        if (pathObj.has_parent_path()) {
            std::error_code ec;
            fs::create_directories(pathObj.parent_path(), ec);
        }
    }

    bool isReservedSpecialToken(const std::string& text) {
        return text == Vocabulary::PAD_TOKEN ||
               text == Vocabulary::UNK_TOKEN ||
               text == Vocabulary::START_TOKEN ||
               text == Vocabulary::END_TOKEN;
    }
}

std::string Exporter::escapeCsvField(const std::string& field, char delimiter) {
    bool needsQuotes = false;
    for (char c : field) {
        if (c == delimiter || c == '"' || c == '\n' || c == '\r' || c == ';') {
            needsQuotes = true;
            break;
        }
    }

    if (!needsQuotes) {
        return field;
    }

    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

bool Exporter::exportTokensToCsv(const Dataset& dataset,
                                 const std::string& filePath,
                                 char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "doc_id" << delimiter
        << "file_path" << delimiter
        << "sentence_number" << delimiter
        << "position" << delimiter
        << "token_text" << delimiter
        << "token_type" << delimiter
        << "token_id\n";

    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            out << doc.id << delimiter
                << escapeCsvField(doc.filePath, delimiter) << delimiter
                << token.sentenceNumber << delimiter
                << token.position << delimiter
                << escapeCsvField(token.text, delimiter) << delimiter
                << escapeCsvField(tokenTypeToString(token.type), delimiter) << delimiter
                << token.tokenId << "\n";
        }
    }

    return out.good();
}

bool Exporter::exportVocabularyToCsv(const Vocabulary& vocab,
                                     const std::string& filePath,
                                     char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "token_id" << delimiter
        << "token_text" << delimiter
        << "corpus_frequency" << delimiter
        << "is_special\n";

    for (size_t id = 0; id < vocab.size(); ++id) {
        int tokenId = static_cast<int>(id);
        std::string text = vocab.getTokenText(tokenId);
        int freq = vocab.getTokenFrequency(text);
        bool special = isReservedSpecialToken(text);

        out << tokenId << delimiter
            << escapeCsvField(text, delimiter) << delimiter
            << freq << delimiter
            << (special ? 1 : 0) << "\n";
    }

    return out.good();
}

bool Exporter::exportEncodedDatasetToCsv(const Dataset& dataset,
                                         const std::string& filePath,
                                         char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "doc_id" << delimiter
        << "file_path" << delimiter
        << "token_count" << delimiter
        << "sequence_length" << delimiter
        << "integer_sequence\n";

    for (const auto& doc : dataset.getDocuments()) {
        std::ostringstream seqStream;
        for (size_t i = 0; i < doc.integerSequence.size(); ++i) {
            if (i > 0) {
                seqStream << ' ';
            }
            seqStream << doc.integerSequence[i];
        }

        out << doc.id << delimiter
            << escapeCsvField(doc.filePath, delimiter) << delimiter
            << doc.tokens.size() << delimiter
            << doc.integerSequence.size() << delimiter
            << escapeCsvField(seqStream.str(), delimiter) << "\n";
    }

    return out.good();
}

bool Exporter::exportBowMatrixToCsv(const Dataset& dataset,
                                    const Vocabulary& vocab,
                                    const std::string& filePath,
                                    char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    size_t vocabSize = vocab.size();

    // Header row: doc_id, file_path, and each vocabulary token column
    out << "doc_id" << delimiter << "file_path";
    for (size_t id = 0; id < vocabSize; ++id) {
        std::string colName = vocab.getTokenText(static_cast<int>(id));
        out << delimiter << escapeCsvField(colName, delimiter);
    }
    out << "\n";

    for (const auto& doc : dataset.getDocuments()) {
        out << doc.id << delimiter << escapeCsvField(doc.filePath, delimiter);
        for (size_t id = 0; id < vocabSize; ++id) {
            int val = (id < doc.bowVector.size()) ? doc.bowVector[id] : 0;
            out << delimiter << val;
        }
        out << "\n";
    }

    return out.good();
}

bool Exporter::exportTfIdfMatrixToCsv(const Dataset& dataset,
                                      const Vocabulary& vocab,
                                      const std::string& filePath,
                                      int precision,
                                      char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    size_t vocabSize = vocab.size();

    // Header row: doc_id, file_path, and each vocabulary token column
    out << "doc_id" << delimiter << "file_path";
    for (size_t id = 0; id < vocabSize; ++id) {
        std::string colName = vocab.getTokenText(static_cast<int>(id));
        out << delimiter << escapeCsvField(colName, delimiter);
    }
    out << "\n";

    out << std::fixed << std::setprecision(precision);
    for (const auto& doc : dataset.getDocuments()) {
        out << doc.id << delimiter << escapeCsvField(doc.filePath, delimiter);
        for (size_t id = 0; id < vocabSize; ++id) {
            double val = (id < doc.tfidfVector.size()) ? doc.tfidfVector[id] : 0.0;
            out << delimiter << val;
        }
        out << "\n";
    }

    return out.good();
}

bool Exporter::exportCorpusStatisticsToCsv(const CorpusStatistics& stats,
                                           const std::string& filePath,
                                           char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "metric" << delimiter << "value\n";
    out << "document_count" << delimiter << stats.documentCount << "\n";
    out << "total_raw_characters" << delimiter << stats.totalRawCharacters << "\n";
    out << "total_token_characters" << delimiter << stats.totalTokenCharacters << "\n";
    out << "total_tokens" << delimiter << stats.totalTokens << "\n";
    out << "unique_tokens" << delimiter << stats.uniqueTokens << "\n";
    out << "in_vocabulary_unique_tokens" << delimiter << stats.inVocabUniqueTokens << "\n";
    out << "unknown_token_occurrences" << delimiter << stats.unknownTokenOccurrences << "\n";
    out << "unique_unknown_tokens" << delimiter << stats.uniqueUnknownTokens << "\n";

    out << std::fixed << std::setprecision(6);
    out << "unknown_token_ratio" << delimiter << stats.unknownTokenRatio << "\n";
    out << "avg_document_length_chars" << delimiter << stats.avgDocumentLengthChars << "\n";
    out << "avg_document_length_tokens" << delimiter << stats.avgDocumentLengthTokens << "\n";
    out << "avg_token_length" << delimiter << stats.avgTokenLength << "\n";
    out << "avg_unique_token_length" << delimiter << stats.avgUniqueTokenLength << "\n";
    out << "avg_unknown_token_length" << delimiter << stats.avgUnknownTokenLength << "\n";

    return out.good();
}

bool Exporter::exportTokenFrequenciesToCsv(const std::vector<TokenFrequencyInfo>& table,
                                           const std::string& filePath,
                                           char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "token_text" << delimiter
        << "token_id" << delimiter
        << "token_type" << delimiter
        << "count" << delimiter
        << "document_frequency" << delimiter
        << "relative_frequency" << delimiter
        << "percentage\n";

    for (const auto& item : table) {
        out << escapeCsvField(item.text, delimiter) << delimiter
            << item.tokenId << delimiter
            << escapeCsvField(tokenTypeToString(item.type), delimiter) << delimiter
            << item.count << delimiter
            << item.documentFrequency << delimiter
            << std::fixed << std::setprecision(6) << item.relativeFrequency << delimiter
            << std::setprecision(4) << item.percentage << "\n";
    }

    return out.good();
}

bool Exporter::exportTokenTypeDistributionToCsv(const std::vector<TokenTypeStats>& distribution,
                                                const std::string& filePath,
                                                char delimiter) {
    ensureParentDirectory(filePath);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "token_type" << delimiter
        << "total_count" << delimiter
        << "unique_count" << delimiter
        << "percentage" << delimiter
        << "avg_length\n";

    for (const auto& item : distribution) {
        out << escapeCsvField(item.typeName, delimiter) << delimiter
            << item.totalCount << delimiter
            << item.uniqueCount << delimiter
            << std::fixed << std::setprecision(4) << item.percentage << delimiter
            << std::setprecision(4) << item.avgLength << "\n";
    }

    return out.good();
}

bool Exporter::exportAllToDirectory(const Dataset& dataset,
                                    const Vocabulary& vocab,
                                    const std::string& outputDir,
                                    char delimiter) {
    std::error_code ec;
    fs::create_directories(outputDir, ec);

    CorpusStatistics stats = StatisticsAnalyzer::analyze(dataset, vocab);
    auto freqTable = StatisticsAnalyzer::getTokenFrequencyTable(dataset, &vocab);
    auto typeDist = StatisticsAnalyzer::getTokenTypeDistribution(dataset);

    bool ok = true;
    ok = exportTokensToCsv(dataset, outputDir + "/tokens.csv", delimiter) && ok;
    ok = exportVocabularyToCsv(vocab, outputDir + "/vocabulary.csv", delimiter) && ok;
    ok = exportEncodedDatasetToCsv(dataset, outputDir + "/encoded_dataset.csv", delimiter) && ok;
    ok = exportBowMatrixToCsv(dataset, vocab, outputDir + "/bow_matrix.csv", delimiter) && ok;
    ok = exportTfIdfMatrixToCsv(dataset, vocab, outputDir + "/tfidf_matrix.csv", 6, delimiter) && ok;
    ok = exportCorpusStatisticsToCsv(stats, outputDir + "/corpus_statistics.csv", delimiter) && ok;
    ok = exportTokenFrequenciesToCsv(freqTable, outputDir + "/token_frequencies.csv", delimiter) && ok;
    ok = exportTokenTypeDistributionToCsv(typeDist, outputDir + "/token_types.csv", delimiter) && ok;

    return ok;
}