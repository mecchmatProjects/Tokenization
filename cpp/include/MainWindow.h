#pragma once
#include <string>
#include <vector>
#include <functional>

// Supported tokenization strategies selectable in the GUI
enum class TokenizerStrategyType {
    Whitespace = 0,
    Basic = 1,
    RuleBased = 2
};

// Converts strategy enum to human-readable label
std::string strategyTypeToString(TokenizerStrategyType strategy);

// Configuration state managed by the graphical user interface
struct GuiConfig {
    std::string inputText =
        "Machine learning is amazing! I have 2 projects.\n"
        "Contact us at test@example.com or visit https://example.com.\n"
        "Price = $19.99, COVID-19 research in C++ is fast!";

    TokenizerStrategyType strategy = TokenizerStrategyType::RuleBased;

    // Case and punctuation preprocessing flags
    bool convertToLowercase = true;
    bool removePunctuation = false;
    bool normalizeWhitespace = true;
    bool preserveMultiCharPunctuation = true;
};

class MainWindow {
public:
    using ConfigChangedCallback = std::function<void(const GuiConfig&)>;

    // Control IDs for GUI elements
    static constexpr int IDC_INPUT_TEXT          = 1001;
    static constexpr int IDC_STRATEGY_COMBO      = 1002;
    static constexpr int IDC_CHK_LOWERCASE       = 1003;
    static constexpr int IDC_CHK_REMOVE_PUNCT    = 1004;
    static constexpr int IDC_CHK_NORM_WHITESPACE = 1005;
    static constexpr int IDC_CHK_MULTI_PUNCT     = 1006;
    static constexpr int IDC_BTN_LOAD_SAMPLE     = 1007;
    static constexpr int IDC_BTN_CLEAR_TEXT      = 1008;
    static constexpr int IDC_BTN_APPLY_CLOSE     = 1009;
    static constexpr int IDC_STATUS_SUMMARY      = 1010;

private:
    GuiConfig config;
    std::string windowTitle;
    int width;
    int height;
    bool isRunning = false;
    void* nativeHwnd = nullptr;
    void* nativeFont = nullptr;
    void* nativeMonoFont = nullptr;
    ConfigChangedCallback onConfigChanged;

public:
    explicit MainWindow(
        const std::string& title = "Text Tokenizer - NLP Preprocessing Studio",
        int windowWidth = 760,
        int windowHeight = 560);

    ~MainWindow();

    // Creates the native window and initializes all child controls
    bool create();

    // Runs the interactive GUI event loop until the window is closed
    int run();

    // Closes the window programmatically
    void close();

    // State accessors and mutators
    const GuiConfig& getConfig() const;
    void setConfig(const GuiConfig& newConfig);

    std::string getInputText() const;
    void setInputText(const std::string& text);

    TokenizerStrategyType getSelectedStrategy() const;
    void setSelectedStrategy(TokenizerStrategyType strategy);

    bool isLowercaseEnabled() const;
    void setLowercaseEnabled(bool enabled);

    bool isRemovePunctuationEnabled() const;
    void setRemovePunctuationEnabled(bool enabled);

    bool isNormalizeWhitespaceEnabled() const;
    void setNormalizeWhitespaceEnabled(bool enabled);

    bool isPreserveMultiCharPunctuationEnabled() const;
    void setPreserveMultiCharPunctuationEnabled(bool enabled);

    void setOnConfigChangedCallback(ConfigChangedCallback callback);

    // Synchronizes state between internal GuiConfig and native GUI controls
    void syncControlsFromConfig();
    void syncConfigFromControls();

    // Builds a formatted summary of the active GUI state
    std::string buildConfigurationSummary() const;

    // Prints current GUI configuration to standard output
    void printConfigReport(const std::string& header = "GUI Configuration State") const;

    // Internal callback handler for native window events
    long long handleWindowMessage(void* hwnd, unsigned int msg, unsigned long long wParam, long long lParam);
};