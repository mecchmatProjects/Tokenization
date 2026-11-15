#include "../include/MainWindow.h"
#include "../include/IntegerEncoder.h"

#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>

namespace {
    void insertListViewColumn(HWND hList, int colIndex, const char* title, int width) {
        LVCOLUMNA col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.cx = width;
        col.pszText = const_cast<char*>(title);
        col.iSubItem = colIndex;
        SendMessageA(hList, LVM_INSERTCOLUMNA, static_cast<WPARAM>(colIndex), reinterpret_cast<LPARAM>(&col));
    }

    int insertListViewRow(HWND hList, int rowIndex, const std::string& firstColText) {
        LVITEMA item = {};
        item.mask = LVIF_TEXT;
        item.iItem = rowIndex;
        item.iSubItem = 0;
        item.pszText = const_cast<char*>(firstColText.c_str());
        return static_cast<int>(SendMessageA(hList, LVM_INSERTITEMA, 0, reinterpret_cast<LPARAM>(&item)));
    }

    void setListViewCellText(HWND hList, int rowIndex, int colIndex, const std::string& text) {
        LVITEMA item = {};
        item.iSubItem = colIndex;
        item.pszText = const_cast<char*>(text.c_str());
        SendMessageA(hList, LVM_SETITEMTEXTA, static_cast<WPARAM>(rowIndex), reinterpret_cast<LPARAM>(&item));
    }

    std::string getEditControlText(HWND hEdit) {
        if (!hEdit) return "";
        int len = GetWindowTextLengthA(hEdit);
        if (len <= 0) return "";
        std::string buffer(static_cast<size_t>(len) + 1, '\0');
        GetWindowTextA(hEdit, &buffer[0], len + 1);
        buffer.resize(static_cast<size_t>(len));
        return buffer;
    }

    int parsePositiveIntOrDefault(const std::string& text, int defaultVal, int minVal) {
        if (text.empty()) return defaultVal;
        try {
            int val = std::stoi(text);
            return (val < minVal) ? minVal : val;
        } catch (...) {
            return defaultVal;
        }
    }

    size_t parseSizeOrDefault(const std::string& text, size_t defaultVal) {
        if (text.empty()) return defaultVal;
        try {
            long long val = std::stoll(text);
            return (val < 0) ? defaultVal : static_cast<size_t>(val);
        } catch (...) {
            return defaultVal;
        }
    }

    LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        MainWindow* self = nullptr;
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCTA* cs = reinterpret_cast<CREATESTRUCTA*>(lParam);
            self = static_cast<MainWindow*>(cs->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            if (self) {
                self->attachNativeHandle(hwnd);
            }
        } else {
            self = reinterpret_cast<MainWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (self) {
            return static_cast<LRESULT>(self->handleMessage(
                static_cast<unsigned int>(uMsg),
                static_cast<unsigned long long>(wParam),
                static_cast<long long>(lParam)));
        }
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}
#endif

MainWindow::MainWindow(const GuiConfig& initialConfig)
    : config(initialConfig) {}

MainWindow::~MainWindow() {
    destroy();
}

bool MainWindow::create(const std::string& title, int width, int height) {
#ifdef _WIN32
    if (initialized && hWnd != nullptr) {
        return true;
    }

    HINSTANCE inst = GetModuleHandleA(nullptr);
    hInstance = inst;

    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    const char* className = "TokenizerStudioMainWindowClass";

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    if (!GetClassInfoExA(inst, className, &wc)) {
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWindowProc;
        wc.hInstance = inst;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        wc.lpszClassName = className;
        RegisterClassExA(&wc);
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW > width) ? (screenW - width) / 2 : 20;
    int posY = (screenH > height) ? (screenH - height) / 2 : 20;

    HWND created = CreateWindowExA(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        posX, posY, width, height,
        nullptr,
        nullptr,
        inst,
        this
    );

    if (!created) {
        return false;
    }

    hWnd = created;
    initialized = true;
    syncControlsFromConfig();
    refreshPreviewTables();
    return true;
#else
    (void)title;
    (void)width;
    (void)height;
    initialized = true;
    return true;
#endif
}

int MainWindow::runMessageLoop() {
#ifdef _WIN32
    if (!hWnd) return -1;

    HWND hwnd = static_cast<HWND>(hWnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return static_cast<int>(msg.wParam);
#else
    return 0;
#endif
}

int MainWindow::showAndRun(const std::string& title, int width, int height) {
    if (!create(title, width, height)) return -1;
    return runMessageLoop();
}

void MainWindow::destroy() {
#ifdef _WIN32
    if (hWnd) {
        DestroyWindow(static_cast<HWND>(hWnd));
        hWnd = nullptr;
    }
    if (hFont) {
        DeleteObject(static_cast<HFONT>(hFont));
        hFont = nullptr;
    }
    if (hBoldFont) {
        DeleteObject(static_cast<HFONT>(hBoldFont));
        hBoldFont = nullptr;
    }
#endif
    initialized = false;
}

void MainWindow::createControls(void* parentPtr) {
#ifdef _WIN32
    HWND parent = static_cast<HWND>(parentPtr);
    HINSTANCE inst = static_cast<HINSTANCE>(hInstance);

    HFONT font = CreateFontA(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI"
    );
    HFONT boldFont = CreateFontA(
        -14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI"
    );
    hFont = font;
    hBoldFont = boldFont;

    auto applyFont = [font, boldFont](HWND ctrl, bool bold = false) {
        if (ctrl) {
            SendMessageA(ctrl, WM_SETFONT, reinterpret_cast<WPARAM>(bold ? boldFont : font), TRUE);
        }
    };

    HWND hGroupInput = CreateWindowExA(
        0, "BUTTON", "1. Input Text Document",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 12, 530, 215, parent, nullptr, inst, nullptr);
    applyFont(hGroupInput, true);

    HWND inputEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        30, 36, 502, 140,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_INPUT_EDIT)), inst, nullptr);
    applyFont(inputEdit);
    hInputEdit = inputEdit;

    HWND hBtnSample = CreateWindowExA(
        0, "BUTTON", "Load Sample Text",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, 184, 150, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_LOAD_SAMPLE)), inst, nullptr);
    applyFont(hBtnSample);

    HWND hBtnClear = CreateWindowExA(
        0, "BUTTON", "Clear Text",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        190, 184, 120, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_CLEAR_TEXT)), inst, nullptr);
    applyFont(hBtnClear);

    HWND hGroupOptions = CreateWindowExA(
        0, "BUTTON", "2. Tokenization Strategy & Preprocessing",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        560, 12, 448, 215, parent, nullptr, inst, nullptr);
    applyFont(hGroupOptions, true);

    HWND hLblStrategy = CreateWindowExA(
        0, "STATIC", "Select Tokenization Strategy:",
        WS_CHILD | WS_VISIBLE,
        576, 38, 220, 20, parent, nullptr, inst, nullptr);
    applyFont(hLblStrategy);

    HWND strategyCombo = CreateWindowExA(
        0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        576, 60, 414, 120,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STRATEGY_COMBO)), inst, nullptr);
    applyFont(strategyCombo);
    hStrategyCombo = strategyCombo;

    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Whitespace Tokenizer"));
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Word/Punctuation (Basic) Tokenizer"));
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Rule-Based Tokenizer"));

    HWND lowercaseCheck = CreateWindowExA(
        0, "BUTTON", "Convert text to lowercase",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        576, 96, 400, 24,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_LOWERCASE)), inst, nullptr);
    applyFont(lowercaseCheck);
    hLowercaseCheck = lowercaseCheck;

    HWND removePunctCheck = CreateWindowExA(
        0, "BUTTON", "Remove punctuation tokens",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        576, 124, 400, 24,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_REMOVE_PUNCT)), inst, nullptr);
    applyFont(removePunctCheck);
    hRemovePunctCheck = removePunctCheck;

    HWND normalizeSpaceCheck = CreateWindowExA(
        0, "BUTTON", "Normalize extra whitespace",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        576, 152, 400, 24,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_NORM_SPACE)), inst, nullptr);
    applyFont(normalizeSpaceCheck);
    hNormalizeSpaceCheck = normalizeSpaceCheck;

    HWND preserveMultiPunctCheck = CreateWindowExA(
        0, "BUTTON", "Preserve multi-character punctuation (... / ?!)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        576, 180, 400, 24,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_MULTI_PUNCT)), inst, nullptr);
    applyFont(preserveMultiPunctCheck);
    hPreserveMultiPunctCheck = preserveMultiPunctCheck;

    HWND hGroupParams = CreateWindowExA(
        0, "BUTTON", "3. Vocabulary & Sequence Alignment Parameters",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 234, 992, 74, parent, nullptr, inst, nullptr);
    applyFont(hGroupParams, true);

    HWND hLblMinFreq = CreateWindowExA(
        0, "STATIC", "minFrequency (>=1):",
        WS_CHILD | WS_VISIBLE,
        30, 264, 130, 22, parent, nullptr, inst, nullptr);
    applyFont(hLblMinFreq);

    HWND minFreqEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "1",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        162, 260, 70, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MIN_FREQ)), inst, nullptr);
    applyFont(minFreqEdit);
    hMinFreqEdit = minFreqEdit;

    HWND hLblMaxVocab = CreateWindowExA(
        0, "STATIC", "maxVocabularySize (0=all):",
        WS_CHILD | WS_VISIBLE,
        250, 264, 175, 22, parent, nullptr, inst, nullptr);
    applyFont(hLblMaxVocab);

    HWND maxVocabEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "0",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        428, 260, 75, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MAX_VOCAB)), inst, nullptr);
    applyFont(maxVocabEdit);
    hMaxVocabEdit = maxVocabEdit;

    HWND hLblMaxLength = CreateWindowExA(
        0, "STATIC", "maxLength (0=dynamic):",
        WS_CHILD | WS_VISIBLE,
        522, 264, 155, 22, parent, nullptr, inst, nullptr);
    applyFont(hLblMaxLength);

    HWND maxLengthEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "0",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        680, 260, 75, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MAX_LENGTH)), inst, nullptr);
    applyFont(maxLengthEdit);
    hMaxLengthEdit = maxLengthEdit;

    HWND hBtnUpdateTables = CreateWindowExA(
        0, "BUTTON", "Update Tables",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        772, 258, 115, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_UPDATE_TABLES)), inst, nullptr);
    applyFont(hBtnUpdateTables, true);

    HWND hBtnResetParams = CreateWindowExA(
        0, "BUTTON", "Reset Params",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        895, 258, 98, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_RESET_PARAMS)), inst, nullptr);
    applyFont(hBtnResetParams);

    HWND hGroupTokens = CreateWindowExA(
        0, "BUTTON", "4. Extracted Tokens & Vocabulary Mapping",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 316, 560, 336, parent, nullptr, inst, nullptr);
    applyFont(hGroupTokens, true);

    HWND tokensTable = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        28, 340, 536, 298,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_TOKENS)), inst, nullptr);
    applyFont(tokensTable);
    hTokensTable = tokensTable;

    HWND hGroupStats = CreateWindowExA(
        0, "BUTTON", "5. Corpus & Vectorization Statistics",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        588, 316, 420, 336, parent, nullptr, inst, nullptr);
    applyFont(hGroupStats, true);

    HWND statsTable = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        600, 340, 396, 298,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_STATS)), inst, nullptr);
    applyFont(statsTable);
    hStatsTable = statsTable;

    initTableColumns();

    HWND hGroupSummary = CreateWindowExA(
        0, "BUTTON", "Active Pipeline Configuration",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 658, 992, 68, parent, nullptr, inst, nullptr);
    applyFont(hGroupSummary, true);

    HWND summaryStatic = CreateWindowExA(
        0, "STATIC", "",
        WS_CHILD | WS_VISIBLE,
        30, 680, 770, 36,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SUMMARY_STATIC)), inst, nullptr);
    applyFont(summaryStatic);
    hSummaryStatic = summaryStatic;

    HWND hBtnApply = CreateWindowExA(
        0, "BUTTON", "Apply Configuration && Close",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        812, 678, 182, 34,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_APPLY_CLOSE)), inst, nullptr);
    applyFont(hBtnApply, true);
#else
    (void)parentPtr;
#endif
}

void MainWindow::initTableColumns() {
#ifdef _WIN32
    if (hTokensTable) {
        HWND list = static_cast<HWND>(hTokensTable);
        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
        SendMessageA(list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, static_cast<LPARAM>(exStyle));

        insertListViewColumn(list, 0, "#", 42);
        insertListViewColumn(list, 1, "Sent", 48);
        insertListViewColumn(list, 2, "Token Text", 145);
        insertListViewColumn(list, 3, "Type", 105);
        insertListViewColumn(list, 4, "Vocab ID", 105);
        insertListViewColumn(list, 5, "Freq", 65);
    }

    if (hStatsTable) {
        HWND list = static_cast<HWND>(hStatsTable);
        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
        SendMessageA(list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, static_cast<LPARAM>(exStyle));

        insertListViewColumn(list, 0, "Metric", 245);
        insertListViewColumn(list, 1, "Value", 125);
    }
#endif
}

long long MainWindow::handleMessage(unsigned int uMsg, unsigned long long wParam, long long lParam) {
#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(hWnd);

    switch (uMsg) {
        case WM_CREATE:
            createControls(hWnd);
            return 0;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == IDC_BTN_LOAD_SAMPLE && code == BN_CLICKED) {
                config.inputText =
                    "Machine learning is amazing! I have 2 projects.\r\n"
                    "Machine learning algorithm and data model = 100$.\r\n"
                    "Machine data is fast.";
                SetWindowTextA(static_cast<HWND>(hInputEdit), config.inputText.c_str());
                syncConfigFromControls();
                updateSummaryText();
                refreshPreviewTables();
                return 0;
            }

            if (id == IDC_BTN_CLEAR_TEXT && code == BN_CLICKED) {
                config.inputText.clear();
                SetWindowTextA(static_cast<HWND>(hInputEdit), "");
                syncConfigFromControls();
                updateSummaryText();
                refreshPreviewTables();
                return 0;
            }

            if (id == IDC_BTN_RESET_PARAMS && code == BN_CLICKED) {
                config.minFrequency = 1;
                config.maxVocabularySize = 0;
                config.maxLength = 0;
                SetWindowTextA(static_cast<HWND>(hMinFreqEdit), "1");
                SetWindowTextA(static_cast<HWND>(hMaxVocabEdit), "0");
                SetWindowTextA(static_cast<HWND>(hMaxLengthEdit), "0");
                syncConfigFromControls();
                updateSummaryText();
                refreshPreviewTables();
                return 0;
            }

            if (id == IDC_BTN_UPDATE_TABLES && code == BN_CLICKED) {
                syncConfigFromControls();
                updateSummaryText();
                refreshPreviewTables();
                return 0;
            }

            if (id == IDC_BTN_APPLY_CLOSE && code == BN_CLICKED) {
                syncConfigFromControls();
                updateSummaryText();
                refreshPreviewTables();
                DestroyWindow(hwnd);
                return 0;
            }

            if ((id == IDC_STRATEGY_COMBO && code == CBN_SELCHANGE) ||
                (id == IDC_CHK_LOWERCASE && code == BN_CLICKED) ||
                (id == IDC_CHK_REMOVE_PUNCT && code == BN_CLICKED) ||
                (id == IDC_CHK_NORM_SPACE && code == BN_CLICKED) ||
                (id == IDC_CHK_MULTI_PUNCT && code == BN_CLICKED)) {
                syncConfigFromControls();
                updateSummaryText();
                refreshPreviewTables();
                return 0;
            }

            if ((id == IDC_INPUT_EDIT || id == IDC_EDIT_MIN_FREQ ||
                 id == IDC_EDIT_MAX_VOCAB || id == IDC_EDIT_MAX_LENGTH) && code == EN_CHANGE) {
                syncConfigFromControls();
                updateSummaryText();
                return 0;
            }
            break;
        }

        case WM_CLOSE:
            syncConfigFromControls();
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            hWnd = nullptr;
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(hwnd, static_cast<UINT>(uMsg), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));
#else
    (void)uMsg;
    (void)wParam;
    (void)lParam;
    return 0;
#endif
}

void MainWindow::syncControlsFromConfig() {
#ifdef _WIN32
    if (!hWnd) return;

    SetWindowTextA(static_cast<HWND>(hInputEdit), config.inputText.c_str());
    SendMessageA(static_cast<HWND>(hStrategyCombo), CB_SETCURSEL, static_cast<WPARAM>(config.strategy), 0);

    SendMessageA(static_cast<HWND>(hLowercaseCheck), BM_SETCHECK, config.lowercase ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hRemovePunctCheck), BM_SETCHECK, config.removePunctuation ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hNormalizeSpaceCheck), BM_SETCHECK, config.normalizeWhitespace ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hPreserveMultiPunctCheck), BM_SETCHECK, config.preserveMultiCharPunctuation ? BST_CHECKED : BST_UNCHECKED, 0);

    SetWindowTextA(static_cast<HWND>(hMinFreqEdit), std::to_string(config.minFrequency).c_str());
    SetWindowTextA(static_cast<HWND>(hMaxVocabEdit), std::to_string(config.maxVocabularySize).c_str());
    SetWindowTextA(static_cast<HWND>(hMaxLengthEdit), std::to_string(config.maxLength).c_str());

    updateSummaryText();
#endif
}

void MainWindow::syncConfigFromControls() {
#ifdef _WIN32
    if (!hWnd) return;

    config.inputText = getEditControlText(static_cast<HWND>(hInputEdit));

    LRESULT sel = SendMessageA(static_cast<HWND>(hStrategyCombo), CB_GETCURSEL, 0, 0);
    if (sel >= 0 && sel <= 2) {
        config.strategy = static_cast<TokenizerStrategyType>(sel);
    }

    config.lowercase = (SendMessageA(static_cast<HWND>(hLowercaseCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.removePunctuation = (SendMessageA(static_cast<HWND>(hRemovePunctCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.normalizeWhitespace = (SendMessageA(static_cast<HWND>(hNormalizeSpaceCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.preserveMultiCharPunctuation = (SendMessageA(static_cast<HWND>(hPreserveMultiPunctCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);

    config.minFrequency = parsePositiveIntOrDefault(getEditControlText(static_cast<HWND>(hMinFreqEdit)), 1, 1);
    config.maxVocabularySize = parseSizeOrDefault(getEditControlText(static_cast<HWND>(hMaxVocabEdit)), 0);
    config.maxLength = parseSizeOrDefault(getEditControlText(static_cast<HWND>(hMaxLengthEdit)), 0);
#endif
}

std::string MainWindow::buildSummaryString() const {
    std::ostringstream oss;
    oss << "Strategy: " << strategyTypeToString(config.strategy)
        << " | Chars: " << config.inputText.size()
        << " | Lowercase: " << (config.lowercase ? "ON" : "OFF")
        << " | RemovePunct: " << (config.removePunctuation ? "ON" : "OFF") << "\r\n"
        << "Parameters: minFrequency=" << config.minFrequency
        << " | maxVocabularySize=" << (config.maxVocabularySize == 0 ? "Unlimited" : std::to_string(config.maxVocabularySize))
        << " | maxLength=" << (config.maxLength == 0 ? "Dynamic" : std::to_string(config.maxLength));
    return oss.str();
}

void MainWindow::updateSummaryText() {
#ifdef _WIN32
    if (hSummaryStatic) {
        std::string summary = buildSummaryString();
        SetWindowTextA(static_cast<HWND>(hSummaryStatic), summary.c_str());
    }
#endif
}

std::vector<Token> MainWindow::previewTokenizeText(const std::string& rawText) const {
    std::vector<Token> tokens;
    size_t pos = 0;
    size_t sentence = 1;
    size_t i = 0;
    const size_t n = rawText.size();

    auto normalizeTokenCase = [this](std::string s) {
        if (config.lowercase) {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        }
        return s;
    };

    if (config.strategy == TokenizerStrategyType::Whitespace) {
        while (i < n) {
            while (i < n && std::isspace(static_cast<unsigned char>(rawText[i]))) {
                ++i;
            }
            if (i >= n) break;
            size_t start = i;
            while (i < n && !std::isspace(static_cast<unsigned char>(rawText[i]))) {
                ++i;
            }
            std::string word = normalizeTokenCase(rawText.substr(start, i - start));
            tokens.emplace_back(word, TokenType::Word, pos++, sentence);
        }
        return tokens;
    }

    while (i < n) {
        unsigned char c = static_cast<unsigned char>(rawText[i]);
        if (std::isspace(c)) {
            ++i;
            continue;
        }

        if (std::isalpha(c)) {
            size_t start = i;
            while (i < n && std::isalpha(static_cast<unsigned char>(rawText[i]))) {
                ++i;
            }
            std::string word = normalizeTokenCase(rawText.substr(start, i - start));
            tokens.emplace_back(word, TokenType::Word, pos++, sentence);
        } else if (std::isdigit(c)) {
            size_t start = i;
            while (i < n && std::isdigit(static_cast<unsigned char>(rawText[i]))) {
                ++i;
            }
            tokens.emplace_back(rawText.substr(start, i - start), TokenType::Number, pos++, sentence);
        } else if (std::ispunct(c)) {
            char ch = rawText[i];
            bool isPunct = (ch == '.' || ch == ',' || ch == '!' || ch == '?' ||
                            ch == ':' || ch == ';' || ch == '(' || ch == ')' ||
                            ch == '[' || ch == ']' || ch == '"' || ch == '\'');
            auto type = isPunct ? TokenType::Punctuation : TokenType::Special;

            std::string sym(1, ch);
            ++i;
            if (config.preserveMultiCharPunctuation && isPunct) {
                while (i < n && (rawText[i] == ch || (ch == '?' && rawText[i] == '!'))) {
                    sym.push_back(rawText[i]);
                    ++i;
                }
            }

            if (type == TokenType::Punctuation && config.removePunctuation) {
                if (ch == '.' || ch == '!' || ch == '?') {
                    ++sentence;
                }
                continue;
            }

            tokens.emplace_back(sym, type, pos++, sentence);
            if (ch == '.' || ch == '!' || ch == '?') {
                ++sentence;
            }
        } else {
            tokens.emplace_back(std::string(1, rawText[i]), TokenType::Unknown, pos++, sentence);
            ++i;
        }
    }

    return tokens;
}

void MainWindow::refreshPreviewTables() {
    Dataset dataset;

    std::istringstream lineStream(config.inputText);
    std::string line;
    int docId = 1;

    while (std::getline(lineStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) continue;

        Document doc;
        doc.id = docId;
        doc.filePath = "input_line_" + std::to_string(docId) + ".txt";
        doc.rawText = line;
        doc.tokens = previewTokenizeText(line);
        dataset.addDocument(doc);
        ++docId;
    }

    Vocabulary vocab;
    vocab.buildFromDataset(dataset, config.minFrequency, config.maxVocabularySize);
    vocab.assignIdsToDataset(dataset);

    IntegerEncoder intEncoder(false, config.maxLength);
    intEncoder.encodeDataset(dataset, vocab);

    CorpusStatistics stats = StatisticsAnalyzer::analyze(dataset, vocab);
    size_t padCount = IntegerEncoder::countDatasetPaddingTokens(dataset);
    double padRatio = IntegerEncoder::calculateDatasetPaddingRatio(dataset);

    populateTokensTableFromDataset(dataset, vocab);
    populateStatisticsTable(stats, vocab.size(), config.maxLength, padCount, padRatio);
}

void MainWindow::populateTokensTable(const std::vector<Token>& tokens, const Vocabulary& vocab) {
#ifdef _WIN32
    if (!hTokensTable) return;
    HWND list = static_cast<HWND>(hTokensTable);

    SendMessageA(list, WM_SETREDRAW, FALSE, 0);
    SendMessageA(list, LVM_DELETEALLITEMS, 0, 0);

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];
        int resolvedId = (t.tokenId >= 0) ? t.tokenId : vocab.getTokenId(t.text);
        int freq = vocab.getTokenFrequency(t.text);

        std::string idDisplay = std::to_string(resolvedId);
        if (resolvedId == Vocabulary::UNK_ID) {
            idDisplay += " (<UNK>)";
        }

        int row = insertListViewRow(list, static_cast<int>(i), std::to_string(t.position));
        setListViewCellText(list, row, 1, std::to_string(t.sentenceNumber));
        setListViewCellText(list, row, 2, t.text);
        setListViewCellText(list, row, 3, tokenTypeToString(t.type));
        setListViewCellText(list, row, 4, idDisplay);
        setListViewCellText(list, row, 5, std::to_string(freq));
    }

    SendMessageA(list, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list, nullptr, TRUE);
#else
    (void)tokens;
    (void)vocab;
#endif
}

void MainWindow::populateTokensTableFromDataset(const Dataset& dataset, const Vocabulary& vocab) {
    std::vector<Token> allTokens;
    for (const auto& doc : dataset.getDocuments()) {
        for (const auto& token : doc.tokens) {
            allTokens.push_back(token);
        }
    }
    populateTokensTable(allTokens, vocab);
}

void MainWindow::populateStatisticsTable(const CorpusStatistics& stats,
                                         size_t vocabSize,
                                         size_t maxSeqLength,
                                         size_t paddingTokenCount,
                                         double paddingRatio) {
#ifdef _WIN32
    if (!hStatsTable) return;
    HWND list = static_cast<HWND>(hStatsTable);

    auto formatDouble = [](double val, int precision = 2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << val;
        return oss.str();
    };

    double avgDocChars = (stats.documentCount > 0)
        ? static_cast<double>(stats.totalRawCharacters) / static_cast<double>(stats.documentCount)
        : 0.0;
    double avgDocTokens = (stats.documentCount > 0)
        ? static_cast<double>(stats.totalTokens) / static_cast<double>(stats.documentCount)
        : 0.0;
    double avgTokenLen = (stats.totalTokens > 0)
        ? static_cast<double>(stats.totalTokenCharacters) / static_cast<double>(stats.totalTokens)
        : 0.0;
    size_t unkOccurrences = static_cast<size_t>(
        std::llround(stats.unknownTokenRatio * static_cast<double>(stats.totalTokens)));

    std::vector<std::pair<std::string, std::string>> rows = {
        {"Documents count", std::to_string(stats.documentCount)},
        {"Total raw characters", std::to_string(stats.totalRawCharacters)},
        {"Total token characters", std::to_string(stats.totalTokenCharacters)},
        {"Total tokens (T)", std::to_string(stats.totalTokens)},
        {"Unique corpus tokens (V_raw)", std::to_string(stats.uniqueTokens)},
        {"Active vocabulary size (|V|)", std::to_string(vocabSize)},
        {"Unknown (<UNK>) occurrences", std::to_string(unkOccurrences)},
        {"Unknown token ratio", formatDouble(stats.unknownTokenRatio * 100.0, 2) + "%"},
        {"Unique unknown tokens", std::to_string(stats.uniqueUnknownTokens)},
        {"Avg doc length (chars)", formatDouble(avgDocChars, 2)},
        {"Avg doc length (tokens)", formatDouble(avgDocTokens, 2)},
        {"Avg token length (chars)", formatDouble(avgTokenLen, 2)},
        {"Sequence maxLength", maxSeqLength == 0 ? "Dynamic (0)" : std::to_string(maxSeqLength)},
        {"Padding (<PAD>) tokens", std::to_string(paddingTokenCount) + " (" + formatDouble(paddingRatio * 100.0, 2) + "%)"}
    };

    SendMessageA(list, WM_SETREDRAW, FALSE, 0);
    SendMessageA(list, LVM_DELETEALLITEMS, 0, 0);

    for (size_t i = 0; i < rows.size(); ++i) {
        int row = insertListViewRow(list, static_cast<int>(i), rows[i].first);
        setListViewCellText(list, row, 1, rows[i].second);
    }

    SendMessageA(list, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list, nullptr, TRUE);
#else
    (void)stats;
    (void)vocabSize;
    (void)maxSeqLength;
    (void)paddingTokenCount;
    (void)paddingRatio;
#endif
}

size_t MainWindow::getTokensTableRowCount() const {
#ifdef _WIN32
    if (!hTokensTable) return 0;
    LRESULT count = SendMessageA(static_cast<HWND>(hTokensTable), LVM_GETITEMCOUNT, 0, 0);
    return (count > 0) ? static_cast<size_t>(count) : 0;
#else
    return 0;
#endif
}

size_t MainWindow::getStatisticsTableRowCount() const {
#ifdef _WIN32
    if (!hStatsTable) return 0;
    LRESULT count = SendMessageA(static_cast<HWND>(hStatsTable), LVM_GETITEMCOUNT, 0, 0);
    return (count > 0) ? static_cast<size_t>(count) : 0;
#else
    return 0;
#endif
}

const GuiConfig& MainWindow::getConfig() const {
    return config;
}

void MainWindow::setConfig(const GuiConfig& newConfig) {
    config = newConfig;
    syncControlsFromConfig();
}