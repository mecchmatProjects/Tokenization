#include <iostream>
#include <string>
#include "../include/MainWindow.h"

int main(int argc, char* argv[]) {
    bool headlessMode = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--headless" || arg == "--no-gui") {
            headlessMode = true;
        }
    }

    std::cout << "=== Tokenizer GUI Framework Verification ===\n\n";

    MainWindow mainWindow("Text Tokenizer - NLP Preprocessing Studio", 760, 560);

    if (!mainWindow.create()) {
        std::cerr << "Failed to initialize native GUI window.\n";
        return 1;
    }

    // 1. Verify initial default GUI state
    mainWindow.printConfigReport("1. Initial Default GUI State");

    // 2. Programmatically test switching strategy and toggling case/punctuation flags
    mainWindow.setSelectedStrategy(TokenizerStrategyType::Basic);
    mainWindow.setLowercaseEnabled(false);
    mainWindow.setRemovePunctuationEnabled(true);
    mainWindow.syncConfigFromControls();
    mainWindow.printConfigReport("2. Programmatic Control Synchronization Check");

    // Restore standard defaults before interactive session
    GuiConfig defaultConfig;
    mainWindow.setConfig(defaultConfig);

    if (headlessMode) {
        std::cout << "Running in headless mode (--headless). Skipping interactive event loop.\n";
        mainWindow.close();
        return 0;
    }

    std::cout << "Launching interactive GUI window...\n";
    std::cout << "(Modify text, select strategy, toggle checkboxes, then click 'Apply & Close')\n\n";

    mainWindow.run();

    // 3. Print final configuration captured from the interactive window
    mainWindow.printConfigReport("3. Final User-Selected GUI Configuration");

    return 0;
}