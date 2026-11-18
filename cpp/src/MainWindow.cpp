#include "../include/MainWindow.h"
#include "../include/IntegerEncoder.h"
#include "../include/Application.h"
#include "preprocessing/TextPreprocessor.h"

#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>

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
    std::wstring utf8ToWide(const std::string& text) {
        if (text.empty()) return {};
        const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                               text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (!length) throw std::runtime_error("Invalid UTF-8 for GUI text");
        std::wstring result(length, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                            static_cast<int>(text.size()), result.data(), length);
        return result;
    }

    void insertListViewColumn(HWND hList, int colIndex, const char* title, int width) {
        LVCOLUMNA col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.cx = width;
        col.pszText = const_cast<char*>(title);
        col.iSubItem = colIndex;
        SendMessageA(hList, LVM_INSERTCOLUMNA, static_cast<WPARAM>(colIndex), reinterpret_cast<LPARAM>(&col));
    }

    int insertListViewRow(HWND hList, int rowIndex, const std::string& firstColText) {
        const auto wide = utf8ToWide(firstColText);
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = rowIndex;
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(wide.c_str());
        return static_cast<int>(SendMessageW(hList, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item)));
    }

    void setListViewCellText(HWND hList, int rowIndex, int colIndex, const std::string& text) {
        const auto wide = utf8ToWide(text);
        LVITEMW item = {};
        item.iSubItem = colIndex;
        item.pszText = const_cast<wchar_t*>(wide.c_str());
        SendMessageW(hList, LVM_SETITEMTEXTW, static_cast<WPARAM>(rowIndex), reinterpret_cast<LPARAM>(&item));
    }

    std::string getEditControlText(HWND hEdit) {
        if (!hEdit) return "";
        int len = GetWindowTextLengthW(hEdit);
        if (len <= 0) return "";
        std::wstring buffer(static_cast<size_t>(len) + 1, L'\0');
        GetWindowTextW(hEdit, &buffer[0], len + 1);
        buffer.resize(static_cast<size_t>(len));
        const int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer.data(),
                                              len, nullptr, 0, nullptr, nullptr);
        if (!bytes) throw std::runtime_error("Invalid Unicode in input");
        std::string result(bytes, '\0');
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer.data(), len,
                            result.data(), bytes, nullptr, nullptr);
        return result;
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
            try {
            return static_cast<LRESULT>(self->handleMessage(
                static_cast<unsigned int>(uMsg),
                static_cast<unsigned long long>(wParam),
                static_cast<long long>(lParam)));
            } catch (const std::exception& error) {
                MessageBoxA(hwnd, error.what(), "Processing error", MB_OK | MB_ICONERROR);
                return 0;
            }
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
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
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
        0, "BUTTON", "Input Text (One Document)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 10, 535, 212, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupInput, true);

    HWND inputEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
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

    // Settings follow the pipeline: preprocessing -> tokenization -> normalization -> morphology.
    auto createGroup = [&](const char* title, int x, int y, int width, int height) {
        HWND group = CreateWindowExA(0, "BUTTON", title,
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            x, y, width, height, parent, nullptr, inst, nullptr);
        applyFont(group, true);
    };
    auto createCheck = [&](const char* title, int id, int x, int y, int width) -> HWND {
        HWND check = CreateWindowExA(0, "BUTTON", title,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
            x, y, width, 22, parent,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), inst, nullptr);
        applyFont(check);
        return check;
    };

    createGroup("1. Text Preprocessing", 564, 10, 484, 116);
    hLowercaseCheck = createCheck("Unicode lowercase", IDC_CHK_LOWERCASE, 580, 34, 210);
    hTrimSpaceCheck = createCheck("Trim text edges", IDC_CHK_TRIM_SPACE, 800, 34, 230);
    hNormalizeSpaceCheck = createCheck("Collapse extra spaces and tabs", IDC_CHK_NORM_SPACE, 580, 58, 440);

    HWND newlineCombo = CreateWindowExA(
        0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        580, 86, 452, 140, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_NEWLINE_COMBO)), inst, nullptr);
    applyFont(newlineCombo);
    hNewlineCombo = newlineCombo;
    SendMessageA(newlineCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Newlines: preserve"));
    SendMessageA(newlineCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Newlines: normalize to LF"));
    SendMessageA(newlineCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Newlines: replace with spaces"));

    createGroup("2. Tokenization", 564, 132, 484, 90);
    HWND strategyCombo = CreateWindowExA(
        0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        580, 156, 452, 140, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STRATEGY_COMBO)), inst, nullptr);
    applyFont(strategyCombo);
    hStrategyCombo = strategyCombo;
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Whitespace (Split by spaces only)"));
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Word/Punctuation (Split words & symbols)"));
    SendMessageA(strategyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Rule-Based (Keep URLs, emails, #tags, C++)"));
    hPreserveMultiPunctCheck = createCheck("Group punctuation sequences (... / ?!)",
        IDC_CHK_MULTI_PUNCT, 580, 190, 450);

    createGroup("3. Token Normalization", 16, 228, 1032, 126);
    hRemovePunctCheck = createCheck("Remove punctuation after tokenization",
        IDC_CHK_REMOVE_PUNCT, 28, 254, 476);

    hReplaceNumberCheck = createCheck("Numbers -> <NUM>", IDC_CHK_REPLACE_NUM, 28, 278, 220);
    hReplaceUrlCheck = createCheck("URLs -> <URL>", IDC_CHK_REPLACE_URL, 260, 278, 240);
    hReplaceEmailCheck = createCheck("Emails -> <EMAIL>", IDC_CHK_REPLACE_EMAIL, 28, 306, 240);
    HWND unicodeCombo = CreateWindowExA(0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        540, 252, 496, 140, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_UNICODE_COMBO)), inst, nullptr);
    applyFont(unicodeCombo);
    hUnicodeCombo = unicodeCombo;
    SendMessageA(unicodeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Unicode: keep original"));
    SendMessageA(unicodeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Unicode: NFC"));
    SendMessageA(unicodeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Unicode: NFKC"));
    hStopWordsCheck = createCheck("Remove stop words", IDC_CHK_STOP_WORDS, 540, 280, 230);
    hStopCaseCheck = createCheck("Case-sensitive stop words", IDC_CHK_STOP_CASE, 780, 280, 256);
    HWND stopLabel = CreateWindowExA(0, "STATIC", "Stop words:", WS_CHILD | WS_VISIBLE,
        540, 312, 90, 22, parent, nullptr, inst, nullptr);
    applyFont(stopLabel);
    HWND stopEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 634, 308, 402, 26, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STOP_WORDS_EDIT)), inst, nullptr);
    applyFont(stopEdit);
    hStopWordsEdit = stopEdit;

    createGroup("4. Morphology (Optional)", 16, 360, 1032, 56);
    hMorphologyCheck = createCheck("English stemming (learning -> learn, models -> model)",
        IDC_CHK_MORPHOLOGY, 28, 384, 990);
    // --- Section 3: Vocabulary & Sequence Alignment Parameters ---
    HWND hGroupParams = CreateWindowExA(
        0, "BUTTON", "5. Vocabulary Filtering && Sequence Alignment Parameters",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 422, 1032, 64, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupParams, true);

    HWND hLblMinFreq = CreateWindowExA(
        0, "STATIC", "Min Word Freq (minFrequency):",
        WS_CHILD | WS_VISIBLE,
        28, 448, 195, 22, parent, nullptr, inst, nullptr
    );
    applyFont(hLblMinFreq);

    HWND minFreqEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "1",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        226, 444, 58, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MIN_FREQ)), inst, nullptr
    );
    applyFont(minFreqEdit);
    hMinFreqEdit = minFreqEdit;

    HWND hLblMaxVocab = CreateWindowExA(
        0, "STATIC", "Vocab Limit (0 = unlimited):",
        WS_CHILD | WS_VISIBLE,
        304, 448, 178, 22, parent, nullptr, inst, nullptr
    );
    applyFont(hLblMaxVocab);

    HWND maxVocabEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "0",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        484, 444, 62, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MAX_VOCAB)), inst, nullptr
    );
    applyFont(maxVocabEdit);
    hMaxVocabEdit = maxVocabEdit;

    HWND hLblMaxLength = CreateWindowExA(
        0, "STATIC", "Fixed Seq Length (0 = auto):",
        WS_CHILD | WS_VISIBLE,
        566, 448, 180, 22, parent, nullptr, inst, nullptr
    );
    applyFont(hLblMaxLength);

    HWND maxLengthEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "0",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        748, 444, 62, 26,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_MAX_LENGTH)), inst, nullptr
    );
    applyFont(maxLengthEdit);
    hMaxLengthEdit = maxLengthEdit;

    HWND hBtnResetParams = CreateWindowExA(
        0, "BUTTON", "Reset Defaults",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        830, 442, 202, 30,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_RESET_PARAMS)), inst, nullptr
    );
    applyFont(hBtnResetParams);

    // --- Section 4: Pipeline Step-by-Step Controls & CSV Export ---
    HWND hGroupActions = CreateWindowExA(
        0, "BUTTON", "6. Pipeline Execution Steps && CSV Export (Click Step-by-Step or Run Full Pipeline)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 492, 1032, 66, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupActions, true);

    HWND hBtnTokenize = CreateWindowExA(
        0, "BUTTON", "1. Tokenize Text",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        28, 516, 175, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_STEP_TOKENIZE)), inst, nullptr
    );
    applyFont(hBtnTokenize, true);

    HWND hBtnVocab = CreateWindowExA(
        0, "BUTTON", "2. Build Vocabulary",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        215, 516, 185, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_STEP_VOCAB)), inst, nullptr
    );
    applyFont(hBtnVocab, true);

    HWND hBtnEncode = CreateWindowExA(
        0, "BUTTON", "3. Encode && Analyze",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        412, 516, 190, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_STEP_ENCODE)), inst, nullptr
    );
    applyFont(hBtnEncode, true);

    HWND hBtnRunAll = CreateWindowExA(
        0, "BUTTON", "Run Full Pipeline (1-3)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        614, 516, 205, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_RUN_PIPELINE)), inst, nullptr
    );
    applyFont(hBtnRunAll, true);

    HWND hBtnExportCsv = CreateWindowExA(
        0, "BUTTON", "4. Export CSV Files",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        831, 516, 201, 32,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_EXPORT_CSV)), inst, nullptr
    );
    applyFont(hBtnExportCsv, true);

    // --- Section 5: Extracted Tokens Table (Bottom-Left) ---
    HWND hGroupTokens = CreateWindowExA(
        0, "BUTTON", "7. Extracted Tokens && Vocabulary Mapping",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 564, 500, 256, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupTokens, true);

    HWND tokensTable = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        28, 588, 476, 220,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_TOKENS)), inst, nullptr
    );
    applyFont(tokensTable);
    hTokensTable = tokensTable;

    // --- Section 6: Corpus & Vectorization Statistics Table (Bottom-Right) ---
    HWND hGroupStats = CreateWindowExA(
        0, "BUTTON", "8. Corpus && Vectorization Summary (With Plain-Language Guide)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        528, 564, 520, 256, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupStats, true);

    HWND statsTable = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        540, 588, 496, 220,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_STATS)), inst, nullptr
    );
    applyFont(statsTable);
    hStatsTable = statsTable;

    initTableColumns();

    // --- Section 7: Live Status Banner & Footer ---
    HWND hGroupSummary = CreateWindowExA(
        0, "BUTTON", "9. Pipeline Status && Active Configuration",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        16, 826, 1032, 68, parent, nullptr, inst, nullptr
    );
    applyFont(hGroupSummary, true);

    HWND summaryStatic = CreateWindowExA(
        0, "STATIC", "",
        WS_CHILD | WS_VISIBLE,
        28, 846, 785, 40,
        parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SUMMARY_STATIC)), inst, nullptr
    );
    applyFont(summaryStatic);
    hSummaryStatic = summaryStatic;

    HWND hBtnApply = CreateWindowExA(
        0, "BUTTON", "Apply Configuration && Close",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        824, 846, 210, 36,
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
            if (syncingControls) return 0;
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == IDC_BTN_LOAD_SAMPLE && code == BN_CLICKED) {
                config.inputText =
                    "Machine learning models are running fast! Contact nlp@university.edu.\r\n"
                    "Natural language processing in C++ handles 100% of #machinelearning tasks.\r\n"
                    "Learning systems and data models improve text processing.";
                SetWindowTextW(static_cast<HWND>(hInputEdit), utf8ToWide(config.inputText).c_str());
                syncConfigFromControls();
                runFullPipeline();
                return 0;
            }

            if (id == IDC_BTN_CLEAR_TEXT && code == BN_CLICKED) {
                config.inputText.clear();
                SetWindowTextW(static_cast<HWND>(hInputEdit), L"");
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

            if (((id == IDC_STRATEGY_COMBO || id == IDC_NEWLINE_COMBO || id == IDC_UNICODE_COMBO) && code == CBN_SELCHANGE) ||
                ((id == IDC_CHK_REPLACE_NUM || id == IDC_CHK_REPLACE_URL || id == IDC_CHK_REPLACE_EMAIL ||
                  id == IDC_CHK_STOP_WORDS || id == IDC_CHK_STOP_CASE) && code == BN_CLICKED) ||
                (id == IDC_CHK_TRIM_SPACE && code == BN_CLICKED) ||
                (id == IDC_CHK_LOWERCASE && code == BN_CLICKED) ||
                (id == IDC_CHK_REMOVE_PUNCT && code == BN_CLICKED) ||
                (id == IDC_CHK_MORPHOLOGY && code == BN_CLICKED) ||
                (id == IDC_CHK_NORM_SPACE && code == BN_CLICKED) ||
                (id == IDC_CHK_MULTI_PUNCT && code == BN_CLICKED)) {
                syncConfigFromControls();
                runFullPipeline();
                return 0;
            }

            if ((id == IDC_INPUT_EDIT || id == IDC_STOP_WORDS_EDIT || id == IDC_EDIT_MIN_FREQ ||
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
    struct SyncGuard {
        bool& flag;
        explicit SyncGuard(bool& value) : flag(value) { flag = true; }
        ~SyncGuard() { flag = false; }
    } guard(syncingControls);

    SetWindowTextW(static_cast<HWND>(hInputEdit), utf8ToWide(config.inputText).c_str());
    SendMessageA(static_cast<HWND>(hStrategyCombo), CB_SETCURSEL, static_cast<WPARAM>(config.strategy), 0);

    SendMessageA(static_cast<HWND>(hLowercaseCheck), BM_SETCHECK, config.lowercase ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hRemovePunctCheck), BM_SETCHECK, config.removePunctuation ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hMorphologyCheck), BM_SETCHECK, config.applyMorphology ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hUnicodeCombo), CB_SETCURSEL, static_cast<WPARAM>(config.normalization.unicode), 0);
    SendMessageA(static_cast<HWND>(hReplaceNumberCheck), BM_SETCHECK, config.normalization.replaceNumbers ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hReplaceUrlCheck), BM_SETCHECK, config.normalization.replaceUrls ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hReplaceEmailCheck), BM_SETCHECK, config.normalization.replaceEmails ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hStopWordsCheck), BM_SETCHECK, config.normalization.removeStopWords ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hStopCaseCheck), BM_SETCHECK, config.normalization.stopWordsCaseSensitive ? BST_CHECKED : BST_UNCHECKED, 0);
    SetWindowTextW(static_cast<HWND>(hStopWordsEdit), utf8ToWide(config.stopWordsText).c_str());
    SendMessageA(static_cast<HWND>(hNormalizeSpaceCheck), BM_SETCHECK, config.normalizeWhitespace ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hTrimSpaceCheck), BM_SETCHECK, config.trimWhitespace ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(static_cast<HWND>(hNewlineCombo), CB_SETCURSEL, static_cast<WPARAM>(config.newlineMode), 0);
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
    auto unicodeSelection = SendMessageA(static_cast<HWND>(hUnicodeCombo), CB_GETCURSEL, 0, 0);
    if (unicodeSelection >= 0 && unicodeSelection <= 2) config.normalization.unicode = static_cast<UnicodeNormalization>(unicodeSelection);
    config.normalization.replaceNumbers = SendMessageA(static_cast<HWND>(hReplaceNumberCheck), BM_GETCHECK, 0, 0) == BST_CHECKED;
    config.normalization.replaceUrls = SendMessageA(static_cast<HWND>(hReplaceUrlCheck), BM_GETCHECK, 0, 0) == BST_CHECKED;
    config.normalization.replaceEmails = SendMessageA(static_cast<HWND>(hReplaceEmailCheck), BM_GETCHECK, 0, 0) == BST_CHECKED;
    config.normalization.removeStopWords = SendMessageA(static_cast<HWND>(hStopWordsCheck), BM_GETCHECK, 0, 0) == BST_CHECKED;
    config.normalization.stopWordsCaseSensitive = SendMessageA(static_cast<HWND>(hStopCaseCheck), BM_GETCHECK, 0, 0) == BST_CHECKED;
    config.stopWordsText = getEditControlText(static_cast<HWND>(hStopWordsEdit));
    config.normalizeWhitespace = (SendMessageA(static_cast<HWND>(hNormalizeSpaceCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.trimWhitespace = (SendMessageA(static_cast<HWND>(hTrimSpaceCheck), BM_GETCHECK, 0, 0) == BST_CHECKED);
    LRESULT newlineSelection = SendMessageA(static_cast<HWND>(hNewlineCombo), CB_GETCURSEL, 0, 0);
    if (newlineSelection >= 0 && newlineSelection <= 2) {
        config.newlineMode = static_cast<NewlineMode>(newlineSelection);
    }
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
        << " | Spaces=" << (config.normalizeWhitespace ? "collapse" : "preserve")
        << " | Trim=" << (config.trimWhitespace ? "ON" : "OFF")
        << " | Newlines=" << (config.newlineMode == NewlineMode::Preserve ? "preserve" :
                               config.newlineMode == NewlineMode::NormalizeToLf ? "LF" : "spaces")
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

std::vector<Token> MainWindow::previewTokenizeText(const std::string& rawText) const {
    PipelineConfig pipeline;
    switch (config.strategy) {
        case TokenizerStrategyType::Whitespace: pipeline.tokenizerType = TokenizerType::Whitespace; break;
        case TokenizerStrategyType::Basic: pipeline.tokenizerType = TokenizerType::WordPunctuation; break;
        case TokenizerStrategyType::RuleBased: pipeline.tokenizerType = TokenizerType::RuleBased; break;
    }
    PreprocessingConfig preprocessing;
    preprocessing.collapseHorizontalWhitespace = config.normalizeWhitespace;
    preprocessing.trimWhitespace = config.trimWhitespace;
    preprocessing.lowercase = config.lowercase;
    preprocessing.newlineMode = config.newlineMode;
    pipeline.preprocessingConfig = preprocessing;
    pipeline.removePunctuation = config.removePunctuation;
    pipeline.normalizationConfig = config.normalization;
    pipeline.normalizationConfig->removePunctuation = config.removePunctuation;
    if (config.normalization.removeStopWords) {
        PreprocessingConfig listOptions;
        listOptions.newlineMode = NewlineMode::ReplaceWithSpace;
        std::istringstream words(TextPreprocessor(listOptions).process(config.stopWordsText));
        std::string word;
        while (words >> word) pipeline.stopWords.add(word);
    }
    pipeline.enableMorphologyNormalization = config.applyMorphology;
    pipeline.preserveMultiCharPunctuation = config.preserveMultiCharPunctuation;
    return Application(pipeline).tokenizeText(rawText);
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

    Document doc;
    doc.id = 1;
    doc.filePath = "input_document.txt";
    doc.rawText = config.inputText;
    doc.tokens = previewTokenizeText(doc.rawText);
    const size_t totalTokensExtracted = doc.tokens.size();
    currentDataset.addDocument(doc);
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
