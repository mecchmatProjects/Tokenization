#include <iostream>
#include <string>
#include "../include/MainWindow.h"

int main(int argc, char* argv[]) {
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--headless") {
            headless = true;
        }
    }

    GuiConfig initialConfig;
    initialConfig.strategy = TokenizerStrategyType::RuleBased;
    initialConfig.lowercase = true;
    initialConfig.removePunctuation = false;
    initialConfig.applyMorphology = false;
    initialConfig.minFrequency = 1;
    initialConfig.maxVocabularySize = 0;
    initialConfig.maxLength = 10;

    MainWindow window(initialConfig);
    if (!window.create("NLP Text Tokenizer & Vectorizer Studio")) {
        std::cerr << "Failed to initialize GUI window.\n";
        return 1;
    }

    std::cout << "=== GUI Pipeline Controls & CSV Export Verification ===\n";

    // Step 1: Tokenize button action
    window.runTokenizationStep();
    std::cout << "[Step 1] " << window.getLastStatusMessage() << "\n";

    // Step 2: Build Vocabulary button action
    window.runBuildVocabularyStep();
    std::cout << "[Step 2] " << window.getLastStatusMessage() << "\n";

    // Step 3: Encode & Analyze button action
    window.runEncodeDatasetStep();
    std::cout << "[Step 3] " << window.getLastStatusMessage() << "\n";

    // Step 4: Export CSV button action (silent in console check, interactive popup on button click)
    bool exported = window.exportToCsv("output", false);
    std::cout << "[Step 4] " << window.getLastStatusMessage()
              << " (success=" << (exported ? "true" : "false") << ")\n";

    std::cout << "Tokens table rows:     " << window.getTokensTableRowCount() << "\n";
    std::cout << "Statistics table rows: " << window.getStatisticsTableRowCount() << "\n\n";

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
