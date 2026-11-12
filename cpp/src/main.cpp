#include <iostream>
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
#include "../include/StatisticsAnalyzer.h"

int main() {
    Dataset dataset = MockData::createSampleDataset();

    Vocabulary fullVocab;
    fullVocab.buildFromDataset(dataset, 1, 0);
    fullVocab.assignIdsToDataset(dataset);

    // 1. General corpus metrics
    CorpusStatistics stats = StatisticsAnalyzer::analyze(dataset, fullVocab);
    StatisticsAnalyzer::printReport(stats, "1. General Corpus Statistics");

    // 2. Full token frequency table sorted by descending frequency
    auto fullFreqTable = StatisticsAnalyzer::getTokenFrequencyTable(dataset, &fullVocab);
    StatisticsAnalyzer::printTokenFrequencyTable(
        fullFreqTable, "2. Full Token Frequency Table (All 18 Unique Tokens)");

    // 3. Top-5 most frequent tokens with filtered vocabulary (minFrequency = 2)
    Vocabulary filteredVocab;
    filteredVocab.buildFromDataset(dataset, 2, 0);
    auto top5Table = StatisticsAnalyzer::getTokenFrequencyTable(dataset, &filteredVocab, 5);
    StatisticsAnalyzer::printTokenFrequencyTable(
        top5Table, "3. Top-5 Frequent Tokens (Filtered Vocab: minFrequency = 2)");

    // 4. Token distribution by TokenType (WORD, NUMBER, PUNCTUATION, SPECIAL, etc.)
    auto typeDistribution = StatisticsAnalyzer::getTokenTypeDistribution(dataset);
    StatisticsAnalyzer::printTokenTypeDistribution(
        typeDistribution, "4. Token Type Distribution");

    return 0;
}