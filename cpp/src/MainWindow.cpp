#include "../include/MainWindow.h"
#include "../include/IntegerEncoder.h"

#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>

#ifdef _WIN32
#define TokenType Win32TokenType_ConflictFix
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#undef TokenType

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

    LRESULT CALLBACK NativeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

namespace {
    std::string escapeCsvField(const std::string& field) {
        bool needsQuotes = false;
        for (char c : field) {
            if (c == ',' || c == '"' || c == '\n' || c == '\r') {
                needsQuotes = true;
                break;
            }
        }
        if (!needsQuotes) return field;
        std::string escaped = "\"";
        for (char c : field) {
            if (c == '"') escaped += "\"\"";
            else escaped += c;
        }
        escaped += "\"";
        return escaped;
    }

    std::string formatDoubleVal(double val, int precision = 2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << val;
        return oss.str();
    }
}

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
        wc.lpfnWndProc = NativeWindowProc;
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

    HWND createdHwnd = CreateWindowExA(
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

    if (!createdHwnd) {
        return false;
    }

    hWnd = createdHwnd;
    initialized = true;
    syncControlsFromConfig();
    runFullPipeline();
    return true;
#else
    (void)title;
    (void)width;
    (void)height;
    initialized = true;
    runFullPipeline();
    return true;
#endif
}

int MainWindow::runMessageLoop() {
#ifdef _WIN32
    if (!hWnd) {
        return -1;
    }

    HWND nativeWnd = static_cast<HWND>(hWnd);
    ShowWindow(nativeWnd, SW_SHOW);
    UpdateWindow(nativeWnd);

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
    if (!create(title, width, height)) {
        return -1;
    }
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

    HFONT regularFont = CreateFontA(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI"
    );
    HFONT boldFont = CreateFontA(
        -14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI"
    );

    hFont = regularFont;
    hBoldFont = boldFont;

    auto applyFont = [regularFont, boldFont](HWND ctrl, bool bold = false) {
        if (ctrl) {
            SendMessageA(ctrl, WM_SETFONT, reinterpret_cast<WPARAM>(bold ? boldFont : regularFont), TRUE);
        }
    };

    // --- Section 1: Input Text Document (Top-Left) ---
    HWND hGroupInput = CreateWindowExA(
        0, "BUTTON", "1. Input Text Documents (One Document per Line)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 10, 535, 212, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupInput, true);

    HWND inputEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        28, 34, 510, 138,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_INPUT_EDIT)), inst, nullptr
    );
    applyFont(inputEdit);
    hInputEdit = inputEdit;

    HWND hBtnSample = CreateWindowExA(
        0, "BUTTON", "Load Sample Corpus",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        28, 180, 165, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_LOAD_SAMPLE)), inst, nullptr
    );
    applyFont(hBtnSample);

    HWND hBtnClear = CreateWindowExA(
        0, "BUTTON", "Clear Text",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        202, 180, 120, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_CLEAR_TEXT)), inst, nullptr
    );
    applyFont(hBtnClear);

    // --- Section 2: Tokenization Strategy & Preprocessing Options (Top-Right) ---
    HWND hGroupOptions = CreateWindowExA(
        0, "BUTTON", "2. Strategy, Case && Morphology Settings",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        564, 10, 484, 212, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupOptions, true);

    HWND hLblStrategy = CreateWindowExA(
        0, "STATIC", "Tokenization Strategy (How text is split):",
        WS_CHILD | WS_VISIBLE,
        580, 34, 300, 20, parent, nullptr, inst, nullptr
    );
    applyFont(hLblStrategy);

    HWND strategyCombo = CreateWindowExA(
        0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        580, 56, 452, 120,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STRATEGY_COMBO)), inst, nullptr
    );
    applyFont(strategyCombo);
    hStrategyCombo = strategyCombo;

    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Whitespace (Split by spaces only)"));
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Word/Punctuation (Split words & symbols)"));
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Rule-Based (Keep URLs, emails, #tags, C++)"));

    HWND lowercaseCheck = CreateWindowExA(
        0, "BUTTON", "Lowercase words (Machine -> machine)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        580, 90, 440, 22,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_LOWERCASE)), inst, nullptr
    );
    applyFont(lowercaseCheck);
    hLowercaseCheck = lowercaseCheck;

    HWND removePunctCheck = CreateWindowExA(
        0, "BUTTON", "Remove punctuation tokens (!, ., ?)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        580, 114, 440, 22,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_REMOVE_PUNCT)), inst, nullptr
    );
    applyFont(removePunctCheck);
    hRemovePunctCheck = removePunctCheck;

    HWND morphologyCheck = CreateWindowExA(
        0, "BUTTON", "Apply morphology stemming (learning -> learn, models -> model)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        580, 138, 455, 22,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_MORPHOLOGY)), inst, nullptr
    );
    applyFont(morphologyCheck);
    hMorphologyCheck = morphologyCheck;

    HWND normalizeSpaceCheck = CreateWindowExA(
        0, "BUTTON", "Normalize extra spaces",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        580, 162, 210, 22,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_NORM_SPACE)), inst, nullptr
    );
    applyFont(normalizeSpaceCheck);
    hNormalizeSpaceCheck = normalizeSpaceCheck;

    HWND preserveMultiPunctCheck = CreateWindowExA(
        0, "BUTTON", "Group multi-punct (... / ?!)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        800, 162, 235, 22,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHK_MULTI_PUNCT)), inst, nullptr
    );
    applyFont(preserveMultiPunctCheck);
    hPreserveMultiPunctCheck = preserveMultiPunctCheck;

    // --- Section 3: Vocabulary & Sequence Alignment Parameters ---
    HWND hGroupParams = CreateWindowExA(
        0, "BUTTON", "3. Vocabulary Filtering && Sequence Alignment Parameters",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 228, 1032, 64, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupParams, true);

    HWND hLblMinFreq = CreateWindowExA(
        0, "STATIC", "Min Word Freq (minFrequency):",
        WS_CHILD | WS_VISIBLE,
        28, 254, 195, 22, parent, nullptr, inst, nullptr
    );
    applyFont(hLblMinFreq);

    HWND minFreqEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "1",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        226, 250, 58, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MIN_FREQ)), inst, nullptr
    );
    applyFont(minFreqEdit);
    hMinFreqEdit = minFreqEdit;

    HWND hLblMaxVocab = CreateWindowExA(
        0, "STATIC", "Vocab Limit (0 = unlimited):",
        WS_CHILD | WS_VISIBLE,
        304, 254, 178, 22, parent, nullptr, inst, nullptr
    );
    applyFont(hLblMaxVocab);

    HWND maxVocabEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "0",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        484, 250, 62, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MAX_VOCAB)), inst, nullptr
    );
    applyFont(maxVocabEdit);
    hMaxVocabEdit = maxVocabEdit;

    HWND hLblMaxLength = CreateWindowExA(
        0, "STATIC", "Fixed Seq Length (0 = auto):",
        WS_CHILD | WS_VISIBLE,
        566, 254, 180, 22, parent, nullptr, inst, nullptr
    );
    applyFont(hLblMaxLength);

    HWND maxLengthEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "0",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        748, 250, 62, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MAX_LENGTH)), inst, nullptr
    );
    applyFont(maxLengthEdit);
    hMaxLengthEdit = maxLengthEdit;

    HWND hBtnResetParams = CreateWindowExA(
        0, "BUTTON", "Reset Defaults",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        830, 248, 202, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_RESET_PARAMS)), inst, nullptr
    );
    applyFont(hBtnResetParams);

    // --- Section 4: Pipeline Step-by-Step Controls & CSV Export ---
    HWND hGroupActions = CreateWindowExA(
        0, "BUTTON", "4. Pipeline Execution Steps && CSV Export (Click Step-by-Step or Run Full Pipeline)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 298, 1032, 66, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupActions, true);

    HWND hBtnTokenize = CreateWindowExA(
        0, "BUTTON", "1. Tokenize Text",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        28, 322, 175, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_STEP_TOKENIZE)), inst, nullptr
    );
    applyFont(hBtnTokenize, true);

    HWND hBtnVocab = CreateWindowExA(
        0, "BUTTON", "2. Build Vocabulary",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        215, 322, 185, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_STEP_VOCAB)), inst, nullptr
    );
    applyFont(hBtnVocab, true);

    HWND hBtnEncode = CreateWindowExA(
        0, "BUTTON", "3. Encode && Analyze",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        412, 322, 190, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_STEP_ENCODE)), inst, nullptr
    );
    applyFont(hBtnEncode, true);

    HWND hBtnRunAll = CreateWindowExA(
        0, "BUTTON", "Run Full Pipeline (1-3)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        614, 322, 205, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_RUN_PIPELINE)), inst, nullptr
    );
    applyFont(hBtnRunAll, true);

    HWND hBtnExportCsv = CreateWindowExA(
        0, "BUTTON", "4. Export CSV Files",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        831, 322, 201, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_EXPORT_CSV)), inst, nullptr
    );
    applyFont(hBtnExportCsv, true);

    // --- Section 5: Extracted Tokens Table (Bottom-Left) ---
    HWND hGroupTokens = CreateWindowExA(
        0, "BUTTON", "5. Extracted Tokens && Vocabulary Mapping",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 370, 500, 326, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupTokens, true);

    HWND tokensTable = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        28, 394, 476, 290,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_TOKENS)), inst, nullptr
    );
    applyFont(tokensTable);
    hTokensTable = tokensTable;

    // --- Section 6: Corpus & Vectorization Statistics Table (Bottom-Right) ---
    HWND hGroupStats = CreateWindowExA(
        0, "BUTTON", "6. Corpus && Vectorization Summary (With Plain-Language Guide)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        528, 370, 520, 326, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupStats, true);

    HWND statsTable = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        540, 394, 496, 290,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_STATS)), inst, nullptr
    );
    applyFont(statsTable);
    hStatsTable = statsTable;

    initTableColumns();

    // --- Section 7: Live Status Banner & Footer ---
    HWND hGroupSummary = CreateWindowExA(
        0, "BUTTON", "7. Pipeline Status && Active Configuration",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 702, 1032, 68, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupSummary, true);

    HWND summaryStatic = CreateWindowExA(
        0, "STATIC", "",
        WS_CHILD | WS_VISIBLE,
        28, 722, 785, 40,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SUMMARY_STATIC)), inst, nullptr
    );
    applyFont(summaryStatic);
    hSummaryStatic = summaryStatic;

    HWND hBtnApply = CreateWindowExA(
        0, "BUTTON", "Apply Configuration && Close",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        824, 722, 210, 36,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_APPLY_CLOSE)), inst, nullptr
    );
    applyFont(hBtnApply, true);
#else
    (void)parentPtr;
#endif
}

void MainWindow::initTableColumns() {
#ifdef _WIN32
    HWND tokensList = static_cast<HWND>(hTokensTable);
    if (tokensList) {
        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
        SendMessageA(tokensList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, static_cast<LPARAM>(exStyle));

        insertListViewColumn(tokensList, 0, "#", 38);
        insertListViewColumn(tokensList, 1, "Sent", 44);
        insertListViewColumn(tokensList, 2, "Token Text", 132);
        insertListViewColumn(tokensList, 3, "Category", 92);
        insertListViewColumn(tokensList, 4, "Vocab ID", 95);
        insertListViewColumn(tokensList, 5, "Freq", 55);
    }

    HWND statsList = static_cast<HWND>(hStatsTable);
    if (statsList) {
        DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
        SendMessageA(statsList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, static_cast<LPARAM>(exStyle));

        insertListViewColumn(statsList, 0, "Metric", 175);
        insertListViewColumn(statsList, 1, "Value", 105);
        insertListViewColumn(statsList, 2, "Meaning / Explanation", 198);
    }
#endif
}

long long MainWindow::handleMessage(unsigned int uMsg, unsigned long long wParam, long long lParam) {
#ifdef _WIN32
    HWND nativeWnd = static_cast<HWND>(hWnd);

    switch (uMsg) {
        case WM_CREATE:
            createControls(nativeWnd);
            return 0;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == IDC_BTN_LOAD_SAMPLE && code == BN_CLICKED) {
                config.inputText =
                    "Machine learning models are running fast! Contact nlp@university.edu.\r\n"
                    "Natural language processing in C++ handles 100% of #machinelearning tasks.\r\n"
                    "Learning systems and data models improve text processing.";
                SetWindowTextA(static_cast<HWND>(hInputEdit), config.inputText.c_str());
                syncConfigFromControls();
                runFullPipeline();
                return 0;
            }

            if (id == IDC_BTN_CLEAR_TEXT && code == BN_CLICKED) {
                config.inputText.clear();
                SetWindowTextA(static_cast<HWND>(hInputEdit), "");
                syncConfigFromControls();
                runFullPipeline();
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
                runFullPipeline();
                return 0;
            }

            if (id == IDC_BTN_STEP_TOKENIZE && code == BN_CLICKED) {
                syncConfigFromControls();
                runTokenizationStep();
                return 0;
            }

            if (id == IDC_BTN_STEP_VOCAB && code == BN_CLICKED) {
                syncConfigFromControls();
                runBuildVocabularyStep();
                return 0;
            }

            if (id == IDC_BTN_STEP_ENCODE && code == BN_CLICKED) {
                syncConfigFromControls();
                runEncodeDatasetStep();
                return 0;
            }

            if ((id == IDC_BTN_RUN_PIPELINE || id == IDC_BTN_UPDATE_TABLES) && code == BN_CLICKED) {
                syncConfigFromControls();
                runFullPipeline();
                return 0;
            }

            if (id == IDC_BTN_EXPORT_CSV && code == BN_CLICKED) {
                syncConfigFromControls();
                exportToCsv(config.exportDirectory, true);
                return 0;
            }

            if (id == IDC_BTN_APPLY_CLOSE && code == BN_CLICKED) {
                syncConfigFromControls();
                runFullPipeline();
                DestroyWindow(nativeWnd);
                return 0;
            }

            if ((id == IDC_STRATEGY_COMBO && code == CBN_SELCHANGE) ||
                (id == IDC_CHK_LOWERCASE && code == BN_CLICKED) ||
                (id == IDC_CHK_REMOVE_PUNCT && code == BN_CLICKED) ||
                (id == IDC_CHK_MORPHOLOGY && code == BN_CLICKED) ||
                (id == IDC_CHK_NORM_SPACE && code == BN_CLICKED) ||
                (id == IDC_CHK_MULTI_PUNCT && code == BN_CLICKED)) {
                syncConfigFromControls();
                runFullPipeline();
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

        case WM_DESTROY:
            hWnd = nullptr;
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(nativeWnd, uMsg, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));
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
    SendMessageA(static_cast<HWND>(hMorphologyCheck), BM_SETCHECK, config.applyMorphology ? BST_CHECKED : BST_UNCHECKED, 0);
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
    config.applyMorphology = (SendMessageA(static_cast<HWND>(hMorphologyCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.normalizeWhitespace = (SendMessageA(static_cast<HWND>(hNormalizeSpaceCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.preserveMultiCharPunctuation = (SendMessageA(static_cast<HWND>(hPreserveMultiPunctCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);

    config.minFrequency = parsePositiveIntOrDefault(getEditControlText(static_cast<HWND>(hMinFreqEdit)), 1, 1);
    config.maxVocabularySize = parseSizeOrDefault(getEditControlText(static_cast<HWND>(hMaxVocabEdit)), 0);
    config.maxLength = parseSizeOrDefault(getEditControlText(static_cast<HWND>(hMaxLengthEdit)), 0);
#endif
}

std::string MainWindow::buildSummaryString() const {
    std::ostringstream oss;
    oss << "Status: " << lastStatusMessage << "\r\n"
        << "Active Config: " << strategyTypeToString(config.strategy)
        << " | Lowercase=" << (config.lowercase ? "ON" : "OFF")
        << " | Morphology=" << (config.applyMorphology ? "ON" : "OFF")
        << " | minFreq=" << config.minFrequency
        << " | maxVocab=" << (config.maxVocabularySize == 0 ? "All" : std::to_string(config.maxVocabularySize))
        << " | maxLength=" << (config.maxLength == 0 ? "Auto" : std::to_string(config.maxLength));
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

std::string MainWindow::stemWord(const std::string& word) {
    if (word.size() <= 3) return word;
    for (char c : word) {
        if (!std::isalpha(static_cast<unsigned char>(c))) return word;
    }

    std::string res = word;
    auto endsWith = [&res](const std::string& suffix) {
        return res.size() > suffix.size() + 2 &&
               res.compare(res.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    if (endsWith("ing")) {
        res.erase(res.size() - 3);
        if (res.size() >= 4 && res[res.size() - 1] == res[res.size() - 2] &&
            res.back() != 'l' && res.back() != 's') {
            res.pop_back();
        }
    } else if (endsWith("ed")) {
        res.erase(res.size() - 2);
    } else if (endsWith("ies") && res.size() > 4) {
        res.replace(res.size() - 3, 3, "y");
    } else if (endsWith("es") && (res.compare(res.size() - 4, 4, "sses") == 0 ||
                                  res.compare(res.size() - 4, 4, "ches") == 0 ||
                                  res.compare(res.size() - 4, 4, "shes") == 0)) {
        res.erase(res.size() - 2);
    } else if (res.size() > 3 && res.back() == 's' && res[res.size() - 2] != 's') {
        res.pop_back();
    }
    return res;
}

std::vector<Token> MainWindow::previewTokenizeText(const std::string& rawText) const {
    std::vector<Token> tokens;
    size_t pos = 0;
    size_t sentence = 1;
    size_t i = 0;
    const size_t n = rawText.size();

    auto normalizeTokenCase = [this](std::string s, bool isWord) {
        if (config.lowercase) {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        }
        if (isWord && config.applyMorphology) {
            s = stemWord(s);
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
            std::string word = normalizeTokenCase(rawText.substr(start, i - start), true);
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

        // Rule-Based special patterns: hashtags (#nlp), emails (user@domain.edu), C++
        if (config.strategy == TokenizerStrategyType::RuleBased) {
            if (rawText[i] == '#' && i + 1 < n && std::isalnum(static_cast<unsigned char>(rawText[i + 1]))) {
                size_t start = i++;
                while (i < n && (std::isalnum(static_cast<unsigned char>(rawText[i])) || rawText[i] == '_')) {
                    ++i;
                }
                tokens.emplace_back(normalizeTokenCase(rawText.substr(start, i - start), false),
                                    TokenType::Special, pos++, sentence);
                continue;
            }

            if (std::isalpha(c)) {
                size_t scan = i;
                while (scan < n && (std::isalnum(static_cast<unsigned char>(rawText[scan])) ||
                                    rawText[scan] == '.' || rawText[scan] == '_' || rawText[scan] == '-')) {
                    ++scan;
                }
                if (scan < n && rawText[scan] == '@') {
                    ++scan;
                    while (scan < n && (std::isalnum(static_cast<unsigned char>(rawText[scan])) ||
                                        rawText[scan] == '.' || rawText[scan] == '-')) {
                        ++scan;
                    }
                    while (scan > i && rawText[scan - 1] == '.') {
                        --scan;
                    }
                    tokens.emplace_back(normalizeTokenCase(rawText.substr(i, scan - i), false),
                                        TokenType::Special, pos++, sentence);
                    i = scan;
                    continue;
                }

                if ((rawText[i] == 'c' || rawText[i] == 'C') && i + 2 < n &&
                    rawText[i + 1] == '+' && rawText[i + 2] == '+') {
                    tokens.emplace_back(normalizeTokenCase(rawText.substr(i, 3), false),
                                        TokenType::Special, pos++, sentence);
                    i += 3;
                    continue;
                }
            }
        }

        if (std::isalpha(c)) {
            size_t start = i;
            while (i < n && std::isalpha(static_cast<unsigned char>(rawText[i]))) {
                ++i;
            }
            std::string word = normalizeTokenCase(rawText.substr(start, i - start), true);
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
            TokenType type = isPunct ? TokenType::Punctuation : TokenType::Special;

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

void MainWindow::computeBowAndTfIdfVectors() {
    size_t vocabSz = currentVocab.size();
    auto& docs = currentDataset.getDocuments();
    size_t docCount = docs.size();

    if (vocabSz == 0 || docCount == 0) return;

    std::vector<size_t> docFreq(vocabSz, 0);

    for (auto& doc : docs) {
        doc.bowVector.assign(vocabSz, 0);
        std::unordered_set<int> uniqueIdsInDoc;
        for (const auto& t : doc.tokens) {
            int id = (t.tokenId >= 0) ? t.tokenId : currentVocab.getTokenId(t.text);
            if (id >= 0 && static_cast<size_t>(id) < vocabSz) {
                doc.bowVector[static_cast<size_t>(id)] += 1;
                uniqueIdsInDoc.insert(id);
            }
        }
        for (int id : uniqueIdsInDoc) {
            docFreq[static_cast<size_t>(id)] += 1;
        }
    }

    for (auto& doc : docs) {
        doc.tfidfVector.assign(vocabSz, 0.0);
        if (doc.tokens.empty()) continue;
        double totalDocTokens = static_cast<double>(doc.tokens.size());
        for (size_t id = 0; id < vocabSz; ++id) {
            if (doc.bowVector[id] > 0 && docFreq[id] > 0) {
                double tf = static_cast<double>(doc.bowVector[id]) / totalDocTokens;
                double idf = std::log(static_cast<double>(docCount) / static_cast<double>(docFreq[id]));
                doc.tfidfVector[id] = tf * idf;
            }
        }
    }
}

void MainWindow::runTokenizationStep() {
    currentDataset = Dataset();
    currentVocab.clear();

    std::istringstream lineStream(config.inputText);
    std::string line;
    int docId = 1;
    size_t totalTokensExtracted = 0;

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
        totalTokensExtracted += doc.tokens.size();
        currentDataset.addDocument(doc);
        ++docId;
    }

    populateTokensTableFromDataset(currentDataset, currentVocab);

    currentStage = PipelineStage::Tokenized;
    lastStatusMessage = "Step 1 (Tokenize) complete: Extracted " + std::to_string(totalTokensExtracted) +
                        " tokens across " + std::to_string(currentDataset.getDocuments().size()) +
                        " document(s). Next: click '2. Build Vocabulary'.";
    updateSummaryText();
}

void MainWindow::runBuildVocabularyStep() {
    if (currentDataset.getDocuments().empty() && !config.inputText.empty()) {
        runTokenizationStep();
    }

    currentVocab.clear();
    currentVocab.buildFromDataset(currentDataset, config.minFrequency, config.maxVocabularySize);
    currentVocab.assignIdsToDataset(currentDataset);

    populateTokensTableFromDataset(currentDataset, currentVocab);

    currentStage = PipelineStage::VocabularyBuilt;
    lastStatusMessage = "Step 2 (Vocabulary) complete: Built active vocabulary |V| = " +
                        std::to_string(currentVocab.size()) + " (minFreq=" +
                        std::to_string(config.minFrequency) + "). Next: click '3. Encode & Analyze'.";
    updateSummaryText();
}

void MainWindow::runEncodeDatasetStep() {
    if (currentVocab.size() <= 4 && !config.inputText.empty()) {
        runBuildVocabularyStep();
    }

    IntegerEncoder intEncoder(false, config.maxLength);
    intEncoder.encodeDataset(currentDataset, currentVocab);
    computeBowAndTfIdfVectors();

    currentStats = StatisticsAnalyzer::analyze(currentDataset, currentVocab);
    currentPadCount = IntegerEncoder::countDatasetPaddingTokens(currentDataset);
    currentPadRatio = IntegerEncoder::calculateDatasetPaddingRatio(currentDataset);

    populateTokensTableFromDataset(currentDataset, currentVocab);
    populateStatisticsTable(currentStats, currentVocab.size(), config.maxLength, currentPadCount, currentPadRatio);

    currentStage = PipelineStage::Encoded;
    lastStatusMessage = "Step 3 (Encode & Analyze) complete: Integer sequences, BoW, TF-IDF & corpus metrics ready. "
                        "Click '4. Export CSV Files' to save.";
    updateSummaryText();
}

void MainWindow::runFullPipeline() {
    runTokenizationStep();
    runBuildVocabularyStep();
    runEncodeDatasetStep();
    lastStatusMessage = "Full pipeline complete (" + std::to_string(currentStats.totalTokens) +
                        " tokens, |V|=" + std::to_string(currentVocab.size()) +
                        " vocab items). Ready to inspect tables or click '4. Export CSV Files'.";
    updateSummaryText();
}

void MainWindow::refreshPreviewTables() {
    runFullPipeline();
}

bool MainWindow::exportToCsv(const std::string& outputDir, bool showPopup) {
    if (currentStage != PipelineStage::Encoded) {
        runFullPipeline();
    }

    try {
        std::filesystem::create_directories(outputDir);

        const std::string tokensPath  = outputDir + "/tokens.csv";
        const std::string vocabPath   = outputDir + "/vocabulary.csv";
        const std::string encodedPath = outputDir + "/encoded_dataset.csv";
        const std::string tfidfPath   = outputDir + "/tfidf_matrix.csv";
        const std::string statsPath   = outputDir + "/statistics.csv";

        // 1. Export tokens.csv
        {
            std::ofstream out(tokensPath);
            if (!out.is_open()) return false;
            out << "doc_id,sentence,position,token_text,token_type,vocab_id,corpus_frequency\n";
            for (const auto& doc : currentDataset.getDocuments()) {
                for (const auto& t : doc.tokens) {
                    int id = (t.tokenId >= 0) ? t.tokenId : currentVocab.getTokenId(t.text);
                    int freq = currentVocab.getTokenFrequency(t.text);
                    out << doc.id << ","
                        << t.sentenceNumber << ","
                        << t.position << ","
                        << escapeCsvField(t.text) << ","
                        << tokenTypeToString(t.type) << ","
                        << id << ","
                        << freq << "\n";
                }
            }
        }

        // 2. Export vocabulary.csv
        {
            std::ofstream out(vocabPath);
            if (!out.is_open()) return false;
            out << "token_id,token_text,frequency,is_special\n";
            for (size_t id = 0; id < currentVocab.size(); ++id) {
                std::string text = currentVocab.getTokenText(static_cast<int>(id));
                int freq = currentVocab.getTokenFrequency(text);
                bool isSpecial = (id < 4);
                out << id << ","
                    << escapeCsvField(text) << ","
                    << freq << ","
                    << (isSpecial ? "true" : "false") << "\n";
            }
        }

        // 3. Export encoded_dataset.csv
        {
            std::ofstream out(encodedPath);
            if (!out.is_open()) return false;
            out << "doc_id,file_path,token_count,sequence_length,integer_sequence,raw_text\n";
            for (const auto& doc : currentDataset.getDocuments()) {
                std::ostringstream seqStream;
                for (size_t i = 0; i < doc.integerSequence.size(); ++i) {
                    if (i > 0) seqStream << " ";
                    seqStream << doc.integerSequence[i];
                }
                out << doc.id << ","
                    << escapeCsvField(doc.filePath) << ","
                    << doc.tokens.size() << ","
                    << doc.integerSequence.size() << ","
                    << escapeCsvField(seqStream.str()) << ","
                    << escapeCsvField(doc.rawText) << "\n";
            }
        }

        // 4. Export tfidf_matrix.csv
        {
            std::ofstream out(tfidfPath);
            if (!out.is_open()) return false;
            out << "doc_id";
            for (size_t id = 0; id < currentVocab.size(); ++id) {
                out << "," << escapeCsvField(currentVocab.getTokenText(static_cast<int>(id)));
            }
            out << "\n";

            for (const auto& doc : currentDataset.getDocuments()) {
                out << doc.id;
                for (size_t id = 0; id < currentVocab.size(); ++id) {
                    double val = (id < doc.tfidfVector.size()) ? doc.tfidfVector[id] : 0.0;
                    out << "," << formatDoubleVal(val, 4);
                }
                out << "\n";
            }
        }

// 5. Export statistics.csv
        {
            std::ofstream out(statsPath);
            if (!out.is_open()) return false;
            double avgTokenLen = (currentStats.totalTokens > 0)
                ? (static_cast<double>(currentStats.totalTokenCharacters) / static_cast<double>(currentStats.totalTokens))
                : 0.0;
            out << "metric,value\n";
            out << "document_count," << currentStats.documentCount << "\n";
            out << "total_tokens," << currentStats.totalTokens << "\n";
            out << "unique_tokens," << currentStats.uniqueTokens << "\n";
            out << "active_vocabulary_size," << currentVocab.size() << "\n";
            out << "unknown_tokens," << currentStats.uniqueUnknownTokens << "\n";
            out << "average_token_length," << formatDoubleVal(avgTokenLen, 2) << "\n";
            out << "sequence_max_length," << config.maxLength << "\n";
            out << "padding_tokens," << currentPadCount << "\n";
            out << "padding_ratio_percent," << formatDoubleVal(currentPadRatio * 100.0, 2) << "\n";
        }

        currentStage = PipelineStage::Exported;
        lastStatusMessage = "Step 4 (Export CSV) complete: Saved 5 CSV files into '" + outputDir +
                            "/' (tokens, vocabulary, encoded_dataset, tfidf_matrix, statistics).";
        updateSummaryText();

#ifdef _WIN32
        if (showPopup && hWnd) {
            std::string msg =
                "Successfully exported all pipeline artifacts to '" + outputDir + "/':\n\n"
                "1. " + tokensPath + " (Token list & IDs)\n"
                "2. " + vocabPath + " (Vocabulary & frequencies)\n"
                "3. " + encodedPath + " (Integer encoded sequences)\n"
                "4. " + tfidfPath + " (TF-IDF weight matrix)\n"
                "5. " + statsPath + " (Corpus summary metrics)";
            MessageBoxA(static_cast<HWND>(hWnd), msg.c_str(), "CSV Export Complete", MB_OK | MB_ICONINFORMATION);
        }
#else
        (void)showPopup;
#endif
        return true;
    } catch (...) {
        lastStatusMessage = "Export error: Unable to write CSV files to '" + outputDir + "/'.";
        updateSummaryText();
        return false;
    }
}

void MainWindow::populateTokensTable(const std::vector<Token>& tokens, const Vocabulary& vocab) {
#ifdef _WIN32
    HWND tokensList = static_cast<HWND>(hTokensTable);
    if (!tokensList) return;

    SendMessageA(tokensList, WM_SETREDRAW, FALSE, 0);
    SendMessageA(tokensList, LVM_DELETEALLITEMS, 0, 0);

    bool vocabReady = (vocab.size() > 4);

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];
        std::string idDisplay = "Pending (Step 2)";
        std::string freqDisplay = "-";

        if (vocabReady) {
            int resolvedId = (t.tokenId >= 0) ? t.tokenId : vocab.getTokenId(t.text);
            int freq = vocab.getTokenFrequency(t.text);
            idDisplay = std::to_string(resolvedId);
            if (resolvedId == Vocabulary::UNK_ID) {
                idDisplay += " (<UNK>)";
            }
            freqDisplay = std::to_string(freq);
        }

        int row = insertListViewRow(tokensList, static_cast<int>(i), std::to_string(t.position));
        setListViewCellText(tokensList, row, 1, std::to_string(t.sentenceNumber));
        setListViewCellText(tokensList, row, 2, t.text);
        setListViewCellText(tokensList, row, 3, tokenTypeToString(t.type));
        setListViewCellText(tokensList, row, 4, idDisplay);
        setListViewCellText(tokensList, row, 5, freqDisplay);
    }

    SendMessageA(tokensList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(tokensList, nullptr, TRUE);
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
    HWND statsList = static_cast<HWND>(hStatsTable);
    if (!statsList) return;

    size_t rawCharCount = 0;
    size_t wordCount = 0;
    size_t numberCount = 0;
    size_t punctCount = 0;
    size_t specialCount = 0;

    for (const auto& doc : currentDataset.getDocuments()) {
        rawCharCount += doc.rawText.size();
        for (const auto& t : doc.tokens) {
            if (t.type == TokenType::Word) ++wordCount;
            else if (t.type == TokenType::Number) ++numberCount;
            else if (t.type == TokenType::Punctuation) ++punctCount;
            else if (t.type == TokenType::Special) ++specialCount;
        }
    }

    double unkRatioPct = stats.unknownTokenRatio * 100.0;
    double avgTokensPerDoc = (stats.documentCount > 0)
        ? (static_cast<double>(stats.totalTokens) / static_cast<double>(stats.documentCount))
        : 0.0;
    double avgTokenLen = (stats.totalTokens > 0)
        ? (static_cast<double>(stats.totalTokenCharacters) / static_cast<double>(stats.totalTokens))
        : 0.0;

    std::string doc1SeqPreview = "None";
    if (!currentDataset.getDocuments().empty()) {
        const auto& seq = currentDataset.getDocuments()[0].integerSequence;
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < seq.size() && i < 8; ++i) {
            if (i > 0) oss << ", ";
            oss << seq[i];
        }
        if (seq.size() > 8) oss << ", ...";
        oss << "]";
        doc1SeqPreview = oss.str();
    }

    struct StatRow {
        std::string metric;
        std::string value;
        std::string explanation;
    };

    std::vector<StatRow> rows = {
        {"Documents count (N)", std::to_string(stats.documentCount), "Lines of text processed as documents"},
        {"Total characters", std::to_string(rawCharCount), "Raw input characters across all docs"},
        {"Total tokens (T)", std::to_string(stats.totalTokens), "All extracted words, numbers & symbols"},
        {"Unique tokens (V_raw)", std::to_string(stats.uniqueTokens), "Distinct tokens before frequency filter"},
        {"Active vocabulary (|V|)", std::to_string(vocabSize), "4 special tags (<PAD>..<END>) + kept words"},
        {"Rare/Unknown (<UNK>)", std::to_string(stats.uniqueUnknownTokens) + " (" + formatDoubleVal(unkRatioPct, 1) + "%)", "Tokens filtered out by minFreq / maxVocab"},
        {"Avg tokens per doc", formatDoubleVal(avgTokensPerDoc, 2), "Average document length in tokens"},
        {"Avg token length", formatDoubleVal(avgTokenLen, 2) + " chars", "Mean length of extracted tokens"},
        {"Word tokens", std::to_string(wordCount), "Alphabetic words (after stemming if ON)"},
        {"Number tokens", std::to_string(numberCount), "Numeric constants (e.g., 100, 42)"},
        {"Punctuation / Special", std::to_string(punctCount) + " / " + std::to_string(specialCount), "Punctuation marks vs emails/#tags/C++"},
        {"Sequence maxLength", maxSeqLength == 0 ? "Auto (Dynamic)" : std::to_string(maxSeqLength), "Fixed vector length for IntegerEncoder"},
        {"Padding (<PAD>) zeros", std::to_string(paddingTokenCount) + " (" + formatDoubleVal(paddingRatio * 100.0, 1) + "%)", "Zero IDs added to align short documents"},
        {"Doc #1 Integer Vector", doc1SeqPreview, "Encoded IDs ready for neural network input"}
    };

    SendMessageA(statsList, WM_SETREDRAW, FALSE, 0);
    SendMessageA(statsList, LVM_DELETEALLITEMS, 0, 0);

    for (size_t i = 0; i < rows.size(); ++i) {
        int row = insertListViewRow(statsList, static_cast<int>(i), rows[i].metric);
        setListViewCellText(statsList, row, 1, rows[i].value);
        setListViewCellText(statsList, row, 2, rows[i].explanation);
    }

    SendMessageA(statsList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(statsList, nullptr, TRUE);
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