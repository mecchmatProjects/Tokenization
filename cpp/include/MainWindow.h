#pragma once
#include <string>
#include <vector>
#include <cstddef>

#include "Token.h"
#include "Dataset.h"
#include "Vocabulary.h"
#include "StatisticsAnalyzer.h"

enum class TokenizerStrategyType {
    Whitespace = 0,
    Basic = 1,
    RuleBased = 2
};

inline std::string strategyTypeToString(TokenizerStrategyType strategy) {
    switch (strategy) {
        case TokenizerStrategyType::Whitespace: return "Whitespace Tokenizer";
        case TokenizerStrategyType::Basic:      return "Word/Punctuation (Basic) Tokenizer";
        case TokenizerStrategyType::RuleBased:  return "Rule-Based Tokenizer";
        default:                                return "Unknown Strategy";
    }
}

struct GuiConfig {
    std::string inputText =
        "Machine learning is amazing! I have 2 projects.\r\n"
        "Machine learning algorithm and data model = 100$.\r\n"
        "Machine data is fast.";
    TokenizerStrategyType strategy = TokenizerStrategyType::Basic;
    bool lowercase = true;
    bool removePunctuation = false;
    bool normalizeWhitespace = true;
    bool preserveMultiCharPunctuation = true;

    int minFrequency = 1;
    size_t maxVocabularySize = 0;
    size_t maxLength = 0;
};

class MainWindow {
public:
    static constexpr int IDC_INPUT_EDIT        = 101;
    static constexpr int IDC_BTN_LOAD_SAMPLE   = 102;
    static constexpr int IDC_BTN_CLEAR_TEXT    = 103;
    static constexpr int IDC_STRATEGY_COMBO    = 104;
    static constexpr int IDC_CHK_LOWERCASE     = 105;
    static constexpr int IDC_CHK_REMOVE_PUNCT  = 106;
    static constexpr int IDC_CHK_NORM_SPACE    = 107;
    static constexpr int IDC_CHK_MULTI_PUNCT   = 108;
    static constexpr int IDC_SUMMARY_STATIC    = 109;
    static constexpr int IDC_BTN_APPLY_CLOSE   = 110;
    static constexpr int IDC_EDIT_MIN_FREQ     = 111;
    static constexpr int IDC_EDIT_MAX_VOCAB    = 112;
    static constexpr int IDC_EDIT_MAX_LENGTH   = 113;
    static constexpr int IDC_BTN_UPDATE_TABLES = 114;
    static constexpr int IDC_BTN_RESET_PARAMS  = 115;
    static constexpr int IDC_LIST_TOKENS       = 116;
    static constexpr int IDC_LIST_STATS        = 117;

private:
    GuiConfig config;
    bool initialized = false;

    void* hInstance = nullptr;
    void* hWnd = nullptr;
    void* hFont = nullptr;
    void* hBoldFont = nullptr;

    void* hInputEdit = nullptr;
    void* hStrategyCombo = nullptr;
    void* hLowercaseCheck = nullptr;
    void* hRemovePunctCheck = nullptr;
    void* hNormalizeSpaceCheck = nullptr;
    void* hPreserveMultiPunctCheck = nullptr;
    void* hSummaryStatic = nullptr;

    void* hMinFreqEdit = nullptr;
    void* hMaxVocabEdit = nullptr;
    void* hMaxLengthEdit = nullptr;

    void* hTokensTable = nullptr;
    void* hStatsTable = nullptr;

    void createControls(void* parent);
    void initTableColumns();
    std::vector<Token> previewTokenizeText(const std::string& rawText) const;

public:
    explicit MainWindow(const GuiConfig& initialConfig = GuiConfig());
    ~MainWindow();

    bool create(const std::string& title = "NLP Text Tokenizer & Vectorizer Studio",
                int width = 1040,
                int height = 780);
    int runMessageLoop();
    int showAndRun(const std::string& title = "NLP Text Tokenizer & Vectorizer Studio",
                   int width = 1040,
                   int height = 780);

    void syncControlsFromConfig();
    void syncConfigFromControls();
    void updateSummaryText();
    void refreshPreviewTables();

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
    std::string buildSummaryString() const;
    void destroy();

    long long handleMessage(unsigned int uMsg, unsigned long long wParam, long long lParam);
    void attachNativeHandle(void* hwnd) { hWnd = hwnd; }
};