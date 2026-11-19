#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../include/Application.h"
#include "../include/MainWindow.h"

namespace {

void printUsage(const char* programName) {
    std::cout
        << "Usage:\n"
        << "  " << programName << "                       Launch interactive GUI window\n"
        << "  " << programName << " --headless            Run GUI pipeline verification and export CSV to ./output\n"
        << "  " << programName << " --file <path> [opts]  Run CLI pipeline on a single text file with timing\n"
        << "  " << programName << " --dir <path>  [opts]  Run CLI pipeline on a directory of .txt files with timing\n\n"
        << "CLI options (for --file / --dir):\n"
        << "  --strategy <whitespace|basic|rulebased>  Tokenization strategy (default: rulebased)\n"
        << "  --no-lower                               Keep original letter case\n"
        << "  --remove-punct                           Strip punctuation tokens\n"
        << "  --no-morph                               Disable morphological normalization\n"
        << "  --min-freq <N>                           Minimum token frequency (default: 1)\n"
        << "  --max-vocab <N>                          Max vocabulary size, 0 = unlimited (default: 0)\n"
        << "  --max-len <N>                            Max sequence length, 0 = dynamic (default: 0)\n"
        << "  --help                                   Show this help message\n";
}

bool parseTokenizerType(const std::string& value, TokenizerType& outType) {
    if (value == "whitespace") {
        outType = TokenizerType::Whitespace;
        return true;
    }
    if (value == "basic" || value == "wordpunct") {
        outType = TokenizerType::WordPunctuation;
        return true;
    }
    if (value == "rulebased") {
        outType = TokenizerType::RuleBased;
        return true;
    }
    return false;
}

int runCliMode(const PipelineConfig& config,
               const std::string& filePath,
               const std::string& dirPath) {
    Application app(config);
    PipelineResult result;
    size_t inputBytes = 0;

    auto startTime = std::chrono::steady_clock::now();

    if (!filePath.empty()) {
        std::ifstream in(filePath, std::ios::binary);
        if (!in) {
            std::cerr << "Error: failed to open input file '" << filePath << "'.\n";
            return 1;
        }
        std::ostringstream buffer;
        buffer << in.rdbuf();
        std::string content = buffer.str();
        inputBytes = content.size();

        result = app.runOnText(content, filePath);
    } else {
        result = app.runOnDirectory(dirPath, ".txt");
        for (const auto& doc : app.getDataset().getDocuments()) {
            inputBytes += doc.rawText.size();
        }
    }

    auto endTime = std::chrono::steady_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    double sizeMb = static_cast<double>(inputBytes) / (1024.0 * 1024.0);
    double throughputMbPerSec = (elapsedMs > 0.0) ? (sizeMb / (elapsedMs / 1000.0)) : 0.0;

    std::cout << "=== CLI Pipeline Benchmark Summary ===\n";
    std::cout << "Source:              " << (!filePath.empty() ? filePath : dirPath) << "\n";
    std::cout << "Strategy:            " << Application::tokenizerTypeToString(config.tokenizerType) << "\n";
    std::cout << "Lowercase / NoPunct: " << (config.convertToLowercase ? "ON" : "OFF")
              << " / " << (config.removePunctuation ? "ON" : "OFF") << "\n";
    std::cout << "Morphology:          " << (config.enableMorphologyNormalization ? "ON" : "OFF") << "\n";
    std::cout << "Documents:           " << result.statistics.documentCount << "\n";
    std::cout << "Input size:          " << std::fixed << std::setprecision(3) << sizeMb
              << " MB (" << inputBytes << " bytes)\n";
    std::cout << "Total tokens (T):    " << result.statistics.totalTokens << "\n";
    std::cout << "Unique tokens:       " << result.statistics.uniqueTokens << "\n";
    std::cout << "Vocabulary size |V|: " << result.vocabularySize << "\n";
    std::cout << "Unknown (<UNK>) %:   " << std::fixed << std::setprecision(2)
              << (result.statistics.unknownTokenRatio * 100.0) << "%\n";
    std::cout << "Padding (<PAD>):     " << result.paddingTokensCount << " ("
              << std::fixed << std::setprecision(2) << (result.paddingRatio * 100.0) << "%)\n";
    std::cout << "Elapsed time:        " << std::fixed << std::setprecision(2) << elapsedMs << " ms\n";
    if (sizeMb > 0.0) {
        std::cout << "Throughput:          " << std::fixed << std::setprecision(2)
                  << throughputMbPerSec << " MB/s\n";
    }

    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    bool headless = false;
    std::string cliFilePath;
    std::string cliDirPath;

    PipelineConfig cliConfig;
    cliConfig.tokenizerType = TokenizerType::RuleBased;
    cliConfig.convertToLowercase = true;
    cliConfig.removePunctuation = false;
    cliConfig.enableMorphologyNormalization = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--headless") {
            headless = true;
        } else if (arg == "--file" && i + 1 < argc) {
            cliFilePath = argv[++i];
        } else if (arg == "--dir" && i + 1 < argc) {
            cliDirPath = argv[++i];
        } else if (arg == "--strategy" && i + 1 < argc) {
            if (!parseTokenizerType(argv[++i], cliConfig.tokenizerType)) {
                std::cerr << "Error: unknown strategy. Use whitespace, basic, or rulebased.\n";
                return 1;
            }
        } else if (arg == "--no-lower") {
            cliConfig.convertToLowercase = false;
        } else if (arg == "--remove-punct") {
            cliConfig.removePunctuation = true;
        } else if (arg == "--no-morph") {
            cliConfig.enableMorphologyNormalization = false;
        } else if (arg == "--min-freq" && i + 1 < argc) {
            try {
                cliConfig.minFrequency = std::max(1, std::stoi(argv[++i]));
            } catch (...) {
                std::cerr << "Error: invalid --min-freq value.\n";
                return 1;
            }
        } else if (arg == "--max-vocab" && i + 1 < argc) {
            try {
                cliConfig.maxVocabularySize = std::stoull(argv[++i]);
            } catch (...) {
                std::cerr << "Error: invalid --max-vocab value.\n";
                return 1;
            }
        } else if (arg == "--max-len" && i + 1 < argc) {
            try {
                cliConfig.maxLength = std::stoull(argv[++i]);
            } catch (...) {
                std::cerr << "Error: invalid --max-len value.\n";
                return 1;
            }
        } else {
            std::cerr << "Error: unrecognized argument '" << arg << "'. Use --help for usage.\n";
            return 1;
        }
    }

    // CLI mode for large-file experiments and T = f(N) performance benchmarks
    if (!cliFilePath.empty() || !cliDirPath.empty()) {
        return runCliMode(cliConfig, cliFilePath, cliDirPath);
    }

    // GUI mode
    GuiConfig initialConfig;
    initialConfig.strategy = TokenizerStrategyType::RuleBased;
    initialConfig.lowercase = true;
    initialConfig.removePunctuation = false;
    initialConfig.applyMorphology = true;
    initialConfig.minFrequency = 1;
    initialConfig.maxVocabularySize = 0;
    initialConfig.maxLength = 10;

    MainWindow window(initialConfig);
    if (!window.create("NLP Text Tokenizer & Vectorizer Studio")) {
        std::cerr << "Failed to initialize GUI window.\n";
        return 1;
    }

    // Populate initial preview tables before showing the window
    window.runTokenizationStep();
    window.runBuildVocabularyStep();
    window.runEncodeDatasetStep();

    if (headless) {
        std::cout << "=== Headless GUI Pipeline & CSV Export Verification ===\n";
        bool exported = window.exportToCsv("output", false);
        std::cout << "Last status:           " << window.getLastStatusMessage()
                  << " (export=" << (exported ? "true" : "false") << ")\n";
        std::cout << "Tokens table rows:     " << window.getTokensTableRowCount() << "\n";
        std::cout << "Statistics table rows: " << window.getStatisticsTableRowCount() << "\n";
        window.destroy();
        return exported ? 0 : 1;
    }

    int exitCode = window.runMessageLoop();

    std::cout << "\n=== Final GUI Session State ===\n";
    std::cout << window.buildSummaryString() << "\n";

    return exitCode;
}