#include "../include/MainWindow.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#endif
#endif

std::string strategyTypeToString(TokenizerStrategyType strategy) {
    switch (strategy) {
        case TokenizerStrategyType::Whitespace:
            return "Whitespace Tokenizer";
        case TokenizerStrategyType::Basic:
            return "Word/Punctuation (Basic) Tokenizer";
        case TokenizerStrategyType::RuleBased:
            return "Rule-Based Tokenizer";
        default:
            return "Unknown Strategy";
    }
}

namespace {

// Converts LF (\n) to CRLF (\r\n) for Win32 multiline EDIT controls
std::string toWin32Newlines(const std::string& text) {
    std::string result;
    result.reserve(text.size() + 16);
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n' && (i == 0 || text[i - 1] != '\r')) {
            result += "\r\n";
        } else {
            result += text[i];
        }
    }
    return result;
}

// Converts CRLF (\r\n) back to standard LF (\n)
std::string fromWin32Newlines(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            continue;
        }
        result += text[i];
    }
    return result;
}

size_t countLines(const std::string& text) {
    if (text.empty()) {
        return 0;
    }
    size_t lines = 1;
    for (char ch : text) {
        if (ch == '\n') {
            ++lines;
        }
    }
    return lines;
}

const std::string kDefaultSampleText =
    "Machine learning is amazing! I have 2 projects.\n"
    "Contact us at test@example.com or visit https://example.com.\n"
    "Price = $19.99, COVID-19 research in C++ is fast!";

#ifdef _WIN32
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* window = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCTA* createStruct = reinterpret_cast<CREATESTRUCTA*>(lParam);
        window = static_cast<MainWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<MainWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return static_cast<LRESULT>(window->handleWindowMessage(
            hwnd,
            static_cast<unsigned int>(msg),
            static_cast<unsigned long long>(wParam),
            static_cast<long long>(lParam)));
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
#endif

} // namespace

MainWindow::MainWindow(const std::string& title, int windowWidth, int windowHeight)
    : windowTitle(title), width(windowWidth), height(windowHeight) {}

MainWindow::~MainWindow() {
#ifdef _WIN32
    if (nativeHwnd) {
        DestroyWindow(static_cast<HWND>(nativeHwnd));
        nativeHwnd = nullptr;
    }
    if (nativeFont) {
        DeleteObject(static_cast<HFONT>(nativeFont));
        nativeFont = nullptr;
    }
    if (nativeMonoFont) {
        DeleteObject(static_cast<HFONT>(nativeMonoFont));
        nativeMonoFont = nullptr;
    }
#endif
}

bool MainWindow::create() {
#ifdef _WIN32
    if (nativeHwnd) {
        return true;
    }

    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    HINSTANCE hInstance = GetModuleHandleA(nullptr);
    const char* className = "TokenizerMainWindowClass";

    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(WNDCLASSEXA);
    if (!GetClassInfoExA(hInstance, className, &wc)) {
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        wc.lpszClassName = className;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        if (!RegisterClassExA(&wc)) {
            return false;
        }
    }

    // Center window on screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW > width) ? (screenW - width) / 2 : CW_USEDEFAULT;
    int posY = (screenH > height) ? (screenH - height) / 2 : CW_USEDEFAULT;

    HWND hwnd = CreateWindowExA(
        0,
        className,
        windowTitle.c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, width, height,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!hwnd) {
        return false;
    }

    nativeHwnd = hwnd;
    syncControlsFromConfig();
    return true;
#else
    return true;
#endif
}

int MainWindow::run() {
    if (!nativeHwnd && !create()) {
        return -1;
    }

#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(nativeHwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    isRunning = true;

    MSG msg{};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    isRunning = false;
    return static_cast<int>(msg.wParam);
#else
    return 0;
#endif
}

void MainWindow::close() {
#ifdef _WIN32
    if (nativeHwnd) {
        syncConfigFromControls();
        DestroyWindow(static_cast<HWND>(nativeHwnd));
        nativeHwnd = nullptr;
    }
#endif
}

const GuiConfig& MainWindow::getConfig() const {
    return config;
}

void MainWindow::setConfig(const GuiConfig& newConfig) {
    config = newConfig;
    syncControlsFromConfig();
}

std::string MainWindow::getInputText() const {
    return config.inputText;
}

void MainWindow::setInputText(const std::string& text) {
    config.inputText = fromWin32Newlines(text);
    syncControlsFromConfig();
}

TokenizerStrategyType MainWindow::getSelectedStrategy() const {
    return config.strategy;
}

void MainWindow::setSelectedStrategy(TokenizerStrategyType strategy) {
    config.strategy = strategy;
    syncControlsFromConfig();
}

bool MainWindow::isLowercaseEnabled() const {
    return config.convertToLowercase;
}

void MainWindow::setLowercaseEnabled(bool enabled) {
    config.convertToLowercase = enabled;
    syncControlsFromConfig();
}

bool MainWindow::isRemovePunctuationEnabled() const {
    return config.removePunctuation;
}

void MainWindow::setRemovePunctuationEnabled(bool enabled) {
    config.removePunctuation = enabled;
    syncControlsFromConfig();
}

bool MainWindow::isNormalizeWhitespaceEnabled() const {
    return config.normalizeWhitespace;
}

void MainWindow::setNormalizeWhitespaceEnabled(bool enabled) {
    config.normalizeWhitespace = enabled;
    syncControlsFromConfig();
}

bool MainWindow::isPreserveMultiCharPunctuationEnabled() const {
    return config.preserveMultiCharPunctuation;
}

void MainWindow::setPreserveMultiCharPunctuationEnabled(bool enabled) {
    config.preserveMultiCharPunctuation = enabled;
    syncControlsFromConfig();
}

void MainWindow::setOnConfigChangedCallback(ConfigChangedCallback callback) {
    onConfigChanged = std::move(callback);
}

void MainWindow::syncControlsFromConfig() {
#ifdef _WIN32
    if (!nativeHwnd) {
        return;
    }
    HWND hwnd = static_cast<HWND>(nativeHwnd);

    HWND hEdit = GetDlgItem(hwnd, IDC_INPUT_TEXT);
    if (hEdit) {
        std::string winText = toWin32Newlines(config.inputText);
        SetWindowTextA(hEdit, winText.c_str());
    }

    HWND hCombo = GetDlgItem(hwnd, IDC_STRATEGY_COMBO);
    if (hCombo) {
        SendMessageA(hCombo, CB_SETCURSEL, static_cast<WPARAM>(config.strategy), 0);
    }

    CheckDlgButton(hwnd, IDC_CHK_LOWERCASE,
                   config.convertToLowercase ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CHK_REMOVE_PUNCT,
                   config.removePunctuation ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CHK_NORM_WHITESPACE,
                   config.normalizeWhitespace ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CHK_MULTI_PUNCT,
                   config.preserveMultiCharPunctuation ? BST_CHECKED : BST_UNCHECKED);

    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS_SUMMARY);
    if (hStatus) {
        std::string summary = toWin32Newlines(buildConfigurationSummary());
        SetWindowTextA(hStatus, summary.c_str());
    }
#endif
}

void MainWindow::syncConfigFromControls() {
#ifdef _WIN32
    if (!nativeHwnd) {
        return;
    }
    HWND hwnd = static_cast<HWND>(nativeHwnd);

    HWND hEdit = GetDlgItem(hwnd, IDC_INPUT_TEXT);
    if (hEdit) {
        int len = GetWindowTextLengthA(hEdit);
        if (len > 0) {
            std::string buffer(static_cast<size_t>(len) + 1, '\0');
            GetWindowTextA(hEdit, &buffer[0], len + 1);
            buffer.resize(static_cast<size_t>(len));
            config.inputText = fromWin32Newlines(buffer);
        } else {
            config.inputText.clear();
        }
    }

    HWND hCombo = GetDlgItem(hwnd, IDC_STRATEGY_COMBO);
    if (hCombo) {
        LRESULT sel = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
        if (sel != CB_ERR && sel >= 0 && sel <= 2) {
            config.strategy = static_cast<TokenizerStrategyType>(sel);
        }
    }

    config.convertToLowercase =
        (IsDlgButtonChecked(hwnd, IDC_CHK_LOWERCASE) == BST_CHECKED);
    config.removePunctuation =
        (IsDlgButtonChecked(hwnd, IDC_CHK_REMOVE_PUNCT) == BST_CHECKED);
    config.normalizeWhitespace =
        (IsDlgButtonChecked(hwnd, IDC_CHK_NORM_WHITESPACE) == BST_CHECKED);
    config.preserveMultiCharPunctuation =
        (IsDlgButtonChecked(hwnd, IDC_CHK_MULTI_PUNCT) == BST_CHECKED);

    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS_SUMMARY);
    if (hStatus) {
        std::string summary = toWin32Newlines(buildConfigurationSummary());
        SetWindowTextA(hStatus, summary.c_str());
    }

    if (onConfigChanged) {
        onConfigChanged(config);
    }
#endif
}

std::string MainWindow::buildConfigurationSummary() const {
    std::ostringstream oss;
    oss << "Active Strategy : " << strategyTypeToString(config.strategy) << "\n"
        << "Input Metrics   : " << config.inputText.size() << " chars | "
        << countLines(config.inputText) << " line(s)\n"
        << "Case Mode       : " << (config.convertToLowercase ? "Convert to Lowercase (case-insensitive)" : "Case-Sensitive (preserve original case)") << "\n"
        << "Punctuation     : " << (config.removePunctuation ? "Strip punctuation tokens" : "Retain punctuation tokens")
        << " | Multi-char (.../?!): " << (config.preserveMultiCharPunctuation ? "Grouped" : "Split") << "\n"
        << "Whitespace      : " << (config.normalizeWhitespace ? "Collapse consecutive spaces" : "Preserve raw whitespace");
    return oss.str();
}

void MainWindow::printConfigReport(const std::string& header) const {
    std::cout << "=== " << header << " ===\n";
    std::cout << "Selected Strategy             : " << strategyTypeToString(config.strategy) << "\n";
    std::cout << "Convert to Lowercase          : " << (config.convertToLowercase ? "ENABLED  [x]" : "DISABLED [ ]") << "\n";
    std::cout << "Remove Punctuation            : " << (config.removePunctuation ? "ENABLED  [x]" : "DISABLED [ ]") << "\n";
    std::cout << "Normalize Extra Whitespace    : " << (config.normalizeWhitespace ? "ENABLED  [x]" : "DISABLED [ ]") << "\n";
    std::cout << "Preserve Multi-Char Punct     : " << (config.preserveMultiCharPunctuation ? "ENABLED  [x]" : "DISABLED [ ]") << "\n";
    std::cout << "Input Text Length (chars)     : " << config.inputText.size() << "\n";
    std::cout << "Input Text Lines              : " << countLines(config.inputText) << "\n";
    std::cout << "Input Text Content:\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << (config.inputText.empty() ? "<EMPTY TEXT>" : config.inputText) << "\n";
    std::cout << "------------------------------------------------------------\n\n";
}

long long MainWindow::handleWindowMessage(void* rawHwnd, unsigned int msg, unsigned long long wParam, long long lParam) {
#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(rawHwnd);

    switch (msg) {
        case WM_CREATE: {
            HINSTANCE hInst = GetModuleHandleA(nullptr);

            HFONT hFont = CreateFontA(
                -15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            HFONT hMonoFont = CreateFontA(
                -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

            nativeFont = hFont;
            nativeMonoFont = hMonoFont;

            // 1. Input Text Section
            HWND hGrpInput = CreateWindowExA(
                0, "BUTTON", "1. Input Text Document",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                16, 12, 710, 195, hwnd, nullptr, hInst, nullptr);

            HWND hEditInput = CreateWindowExA(
                WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                30, 36, 682, 122,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_INPUT_TEXT)), hInst, nullptr);

            HWND hBtnSample = CreateWindowExA(
                0, "BUTTON", "Load Sample Text",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                30, 166, 150, 28,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_LOAD_SAMPLE)), hInst, nullptr);

            HWND hBtnClear = CreateWindowExA(
                0, "BUTTON", "Clear Text",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                190, 166, 120, 28,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_CLEAR_TEXT)), hInst, nullptr);

            // 2. Strategy & Case/Punctuation Settings Section
            HWND hGrpConfig = CreateWindowExA(
                0, "BUTTON", "2. Tokenization Strategy && Case / Punctuation Settings",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                16, 218, 710, 142, hwnd, nullptr, hInst, nullptr);

            HWND hLblStrategy = CreateWindowExA(
                0, "STATIC", "Tokenization Strategy:",
                WS_CHILD | WS_VISIBLE,
                32, 248, 160, 22, hwnd, nullptr, hInst, nullptr);

            HWND hComboStrategy = CreateWindowExA(
                0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                195, 244, 310, 120,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STRATEGY_COMBO)), hInst, nullptr);

            SendMessageA(hComboStrategy, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Whitespace Tokenizer"));
            SendMessageA(hComboStrategy, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Word/Punctuation (Basic) Tokenizer"));
            SendMessageA(hComboStrategy, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Rule-Based Tokenizer"));

            HWND hChkLower = CreateWindowExA(
                0, "BUTTON", "Convert to lowercase (case-insensitive)",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                32, 284, 320, 24,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_LOWERCASE)), hInst, nullptr);

            HWND hChkPunct = CreateWindowExA(
                0, "BUTTON", "Remove punctuation tokens",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                370, 284, 320, 24,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_REMOVE_PUNCT)), hInst, nullptr);

            HWND hChkSpace = CreateWindowExA(
                0, "BUTTON", "Normalize extra whitespace",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                32, 316, 320, 24,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_NORM_WHITESPACE)), hInst, nullptr);

            HWND hChkMultiPunct = CreateWindowExA(
                0, "BUTTON", "Preserve multi-char punctuation (... / ?!)",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                370, 316, 320, 24,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_MULTI_PUNCT)), hInst, nullptr);

            // 3. Live Configuration Summary Panel
            HWND hGrpStatus = CreateWindowExA(
                0, "BUTTON", "3. Active Pipeline Configuration Summary",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                16, 370, 710, 140, hwnd, nullptr, hInst, nullptr);

            HWND hStatusText = CreateWindowExA(
                WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
                30, 394, 495, 98,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STATUS_SUMMARY)), hInst, nullptr);

            HWND hBtnApply = CreateWindowExA(
                0, "BUTTON", "Apply && Close",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                540, 424, 170, 42,
                hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_APPLY_CLOSE)), hInst, nullptr);

            // Apply fonts to controls
            HWND uiControls[] = {
                hGrpInput, hBtnSample, hBtnClear, hGrpConfig, hLblStrategy,
                hComboStrategy, hChkLower, hChkPunct, hChkSpace, hChkMultiPunct,
                hGrpStatus, hBtnApply
            };
            for (HWND ctrl : uiControls) {
                if (ctrl && hFont) {
                    SendMessageA(ctrl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                }
            }
            if (hEditInput && hMonoFont) {
                SendMessageA(hEditInput, WM_SETFONT, reinterpret_cast<WPARAM>(hMonoFont), TRUE);
            }
            if (hStatusText && hMonoFont) {
                SendMessageA(hStatusText, WM_SETFONT, reinterpret_cast<WPARAM>(hMonoFont), TRUE);
            }

            return 0;
        }

        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            int notifyCode = HIWORD(wParam);

            if (controlId == IDC_BTN_LOAD_SAMPLE && notifyCode == BN_CLICKED) {
                config.inputText = kDefaultSampleText;
                syncControlsFromConfig();
                return 0;
            }

            if (controlId == IDC_BTN_CLEAR_TEXT && notifyCode == BN_CLICKED) {
                config.inputText.clear();
                syncControlsFromConfig();
                return 0;
            }

            if (controlId == IDC_BTN_APPLY_CLOSE && notifyCode == BN_CLICKED) {
                syncConfigFromControls();
                DestroyWindow(hwnd);
                return 0;
            }

            if ((controlId == IDC_INPUT_TEXT && notifyCode == EN_CHANGE) ||
                (controlId == IDC_STRATEGY_COMBO && notifyCode == CBN_SELCHANGE) ||
                ((controlId == IDC_CHK_LOWERCASE ||
                  controlId == IDC_CHK_REMOVE_PUNCT ||
                  controlId == IDC_CHK_NORM_WHITESPACE ||
                  controlId == IDC_CHK_MULTI_PUNCT) && notifyCode == BN_CLICKED)) {
                syncConfigFromControls();
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            syncConfigFromControls();
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY: {
            nativeHwnd = nullptr;
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
#else
    (void)rawHwnd;
    (void)msg;
    (void)wParam;
    (void)lParam;
    return 0;
#endif
}