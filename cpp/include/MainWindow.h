#pragma once
#include <string>
#include <vector>
#include <cstddef>

#include "Token.h"
#include "Dataset.h"
#include "Vocabulary.h"
#include "StatisticsAnalyzer.h"
#include "preprocessing/PreprocessingConfig.h"
#include "normalization/NormalizationConfig.h"

enum class TokenizerStrategyType {
    Whitespace = 0,
    Basic = 1,
    RuleBased = 2
};

inline std::string strategyTypeToString(TokenizerStrategyType strategy) {
    switch (strategy) {
        case TokenizerStrategyType::Whitespace: return "Whitespace (Split by spaces)";
        case TokenizerStrategyType::Basic:      return "Word/Punctuation (Basic split)";
        case TokenizerStrategyType::RuleBased:  return "Rule-Based (Keep URLs, emails, #tags, C++)";
        default:                                return "Unknown Strategy";
    }
}

enum class PipelineStage {
    Idle = 0,
    Tokenized = 1,
    VocabularyBuilt = 2,
    Encoded = 3,
    Exported = 4
};

struct GuiConfig {
    std::string inputText =
        "Machine learning models are running fast! Contact nlp@university.edu.\r\n"
        "Natural language processing in C++ handles 100% of #machinelearning tasks.\r\n"
        "Learning systems and data models improve text processing.";
    TokenizerStrategyType strategy = TokenizerStrategyType::RuleBased;
    bool lowercase = true;
    bool removePunctuation = false;
    bool normalizeWhitespace = true;
    bool trimWhitespace = true;
    NewlineMode newlineMode = NewlineMode::ReplaceWithSpace;
    bool preserveMultiCharPunctuation = true;
    bool applyMorphology = false;
    NormalizationConfig normalization;
    std::string stopWordsText = "the a an is of to and";

    int minFrequency = 1;
    size_t maxVocabularySize = 0;
    size_t maxLength = 0;

    std::string exportDirectory = "output";
};

class MainWindow {
public:
    // Input & preprocessing control IDs
    static constexpr int IDC_INPUT_EDIT          = 101;
    static constexpr int IDC_BTN_LOAD_SAMPLE     = 102;
    static constexpr int IDC_BTN_CLEAR_TEXT      = 103;
    static constexpr int IDC_STRATEGY_COMBO      = 104;
    static constexpr int IDC_CHK_LOWERCASE       = 105;
    static constexpr int IDC_CHK_REMOVE_PUNCT    = 106;
    static constexpr int IDC_CHK_NORM_SPACE      = 107;
    static constexpr int IDC_CHK_MULTI_PUNCT     = 108;
    static constexpr int IDC_SUMMARY_STATIC      = 109;
    static constexpr int IDC_BTN_APPLY_CLOSE     = 110;

    // Parameter controls & tables
    static constexpr int IDC_EDIT_MIN_FREQ       = 111;
    static constexpr int IDC_EDIT_MAX_VOCAB      = 112;
    static constexpr int IDC_EDIT_MAX_LENGTH     = 113;
    static constexpr int IDC_BTN_UPDATE_TABLES   = 114;
    static constexpr int IDC_BTN_RESET_PARAMS    = 115;
    static constexpr int IDC_LIST_TOKENS         = 116;
    static constexpr int IDC_LIST_STATS          = 117;

    // Pipeline stage & export controls
    static constexpr int IDC_CHK_MORPHOLOGY      = 118;
    static constexpr int IDC_BTN_STEP_TOKENIZE   = 119;
    static constexpr int IDC_BTN_STEP_VOCAB      = 120;
    static constexpr int IDC_BTN_STEP_ENCODE     = 121;
    static constexpr int IDC_BTN_RUN_PIPELINE    = 122;
    static constexpr int IDC_BTN_EXPORT_CSV      = 123;
    static constexpr int IDC_CHK_TRIM_SPACE      = 124;
    static constexpr int IDC_NEWLINE_COMBO       = 125;
    static constexpr int IDC_UNICODE_COMBO       = 126;
    static constexpr int IDC_CHK_REPLACE_NUM      = 127;
    static constexpr int IDC_CHK_REPLACE_URL      = 128;
    static constexpr int IDC_CHK_REPLACE_EMAIL    = 129;
    static constexpr int IDC_CHK_STOP_WORDS       = 130;
    static constexpr int IDC_CHK_STOP_CASE        = 131;
    static constexpr int IDC_STOP_WORDS_EDIT      = 132;

private:
    GuiConfig config;
    bool initialized = false;
    bool syncingControls = false;
    PipelineStage currentStage = PipelineStage::Idle;
    std::string lastStatusMessage = "Ready. Click '1. Tokenize' for step-by-step mode or 'Run Full Pipeline'.";

    // Active pipeline data artifacts
    Dataset currentDataset;
    Vocabulary currentVocab;
    CorpusStatistics currentStats{};
    size_t currentPadCount = 0;
    double currentPadRatio = 0.0;

    // Opaque Win32 handles (isolated from header to prevent macro collisions)
    void* hInstance = nullptr;
    void* hWnd = nullptr;
    void* hFont = nullptr;
    void* hBoldFont = nullptr;

    void* hInputEdit = nullptr;
    void* hStrategyCombo = nullptr;
    void* hLowercaseCheck = nullptr;
    void* hRemovePunctCheck = nullptr;
    void* hNormalizeSpaceCheck = nullptr;
    void* hTrimSpaceCheck = nullptr;
    void* hNewlineCombo = nullptr;
    void* hPreserveMultiPunctCheck = nullptr;
    void* hMorphologyCheck = nullptr;
    void* hUnicodeCombo = nullptr;
    void* hReplaceNumberCheck = nullptr;
    void* hReplaceUrlCheck = nullptr;
    void* hReplaceEmailCheck = nullptr;
    void* hStopWordsCheck = nullptr;
    void* hStopCaseCheck = nullptr;
    void* hStopWordsEdit = nullptr;
    void* hSummaryStatic = nullptr;

    void* hMinFreqEdit = nullptr;
    void* hMaxVocabEdit = nullptr;
    void* hMaxLengthEdit = nullptr;

    void* hTokensTable = nullptr;
    void* hStatsTable = nullptr;

    void createControls(void* parent);
    void initTableColumns();
    std::vector<Token> previewTokenizeText(const std::string& rawText) const;
    void computeBowAndTfIdfVectors();

public:
    explicit MainWindow(const GuiConfig& initialConfig = GuiConfig());
    ~MainWindow();

    bool create(const std::string& title = "NLP Text Tokenizer & Vectorizer Studio",
                int width = 1080,
                int height = 944);
    int runMessageLoop();
    int showAndRun(const std::string& title = "NLP Text Tokenizer & Vectorizer Studio",
                   int width = 1080,
                   int height = 944);

    // Synchronization between UI controls and GuiConfig
    void syncControlsFromConfig();
    void syncConfigFromControls();
    void updateSummaryText();
    void refreshPreviewTables();

    // Interactive pipeline step handlers
    void runTokenizationStep();
    void runBuildVocabularyStep();
    void runEncodeDatasetStep();
    void runFullPipeline();
    bool exportToCsv(const std::string& outputDir = "output", bool showPopup = false);

    // Table population methods
    void populateTokensTable(const std::vector<Token>& tokens, const Vocabulary& vocab);
    void populateTokensTableFromDataset(const Dataset& dataset, const Vocabulary& vocab);
    void populateStatisticsTable(const CorpusStatistics& stats,
                                 size_t vocabSize,
                                 size_t maxSeqLength = 0,
                                 size_t paddingTokenCount = 0,
                                 double paddingRatio = 0.0);

    size_t getTokensTableRowCount() const;
    size_t getStatisticsTableRowCount() const;

    const GuiConfig& getConfig() const;
    void setConfig(const GuiConfig& newConfig);
    PipelineStage getCurrentStage() const { return currentStage; }
    const std::string& getLastStatusMessage() const { return lastStatusMessage; }
    const Dataset& getCurrentDataset() const { return currentDataset; }
    const Vocabulary& getCurrentVocabulary() const { return currentVocab; }

    std::string buildSummaryString() const;
    void destroy();

    long long handleMessage(unsigned int uMsg, unsigned long long wParam, long long lParam);
    void attachNativeHandle(void* hwnd) { hWnd = hwnd; }
};
