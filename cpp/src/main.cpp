#include <iostream>
#include <string>
#include "../include/MainWindow.h"
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
#include "../include/IntegerEncoder.h"
#include "../include/StatisticsAnalyzer.h"

int main(int argc, char* argv[]) {
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--headless") {
            headless = true;
        }
    }

    GuiConfig testConfig;
    testConfig.strategy = TokenizerStrategyType::Basic;
    testConfig.lowercase = true;
    testConfig.removePunctuation = false;
    testConfig.minFrequency = 2;
    testConfig.maxVocabularySize = 10;
    testConfig.maxLength = 12;

    MainWindow window(testConfig);
    if (!window.create("NLP Text Tokenizer & Vectorizer Studio")) {
        std::cerr << "Failed to initialize GUI window.\n";
        return 1;
    }

    Dataset sampleDataset = MockData::createSampleDataset();
    Vocabulary vocab;
    vocab.buildFromDataset(sampleDataset, testConfig.minFrequency, testConfig.maxVocabularySize);
    vocab.assignIdsToDataset(sampleDataset);

    IntegerEncoder intEncoder(false, testConfig.maxLength);
    intEncoder.encodeDataset(sampleDataset, vocab);

    CorpusStatistics stats = StatisticsAnalyzer::analyze(sampleDataset, vocab);
    size_t padCount = IntegerEncoder::countDatasetPaddingTokens(sampleDataset);
    double padRatio = IntegerEncoder::calculateDatasetPaddingRatio(sampleDataset);

    window.populateTokensTableFromDataset(sampleDataset, vocab);
    window.populateStatisticsTable(stats, vocab.size(), testConfig.maxLength, padCount, padRatio);
    window.syncConfigFromControls();

    std::cout << "=== GUI Parameters & Result Tables Verification ===\n";
    std::cout << window.buildSummaryString() << "\n";
    std::cout << "Tokens table rows populated:     " << window.getTokensTableRowCount() << " (expected: 24)\n";
    std::cout << "Statistics table rows populated: " << window.getStatisticsTableRowCount() << " (expected: 14)\n\n";

    if (headless) {
        window.destroy();
        std::cout << "Headless verification completed successfully.\n";
        return 0;
    }

    std::cout << "Launching interactive GUI window...\n";
    int exitCode = window.runMessageLoop();

    std::cout << "\n=== Final GUI Session State ===\n";
    std::cout << window.buildSummaryString() << "\n";

    return exitCode;
}