#include "Application.h"
#include "MainWindow.h"

#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

int main() {
    try {
        PipelineConfig config;

        config.enableMorphologyNormalization = false;
        Application application(config);
        require(application.preprocessText(u8"  ПРИВІТ\r\n WORLD  ") == u8"привіт world",
                "Application must use Unicode preprocessing");

        config.normalizeWhitespace = false;
        config.convertToLowercase = false;
        application.setConfig(config);
        const std::string text = " \tHello\r\n World ";
        require(application.preprocessText(text) == text,
                "Disabled preprocessing must preserve whitespace and case");

        config.normalizeWhitespace = true;
        config.convertToLowercase = true;
        config.removePunctuation = true;
        config.tokenizerType = TokenizerType::RuleBased;
        config.replaceUrlsAndEmailsWithMarker = true;
        application.setConfig(config);
        require(application.preprocessText("https://example.com user@example.com!") ==
                    "https://example.com user@example.com!",
                "Preprocessing must preserve punctuation for recognition");
        const auto tokens = application.tokenizeText("Visit https://example.com! user@example.com.");
        require(tokens.size() == 3 && tokens[0].text == "visit" &&
                    tokens[1].text == "<URL>" && tokens[2].text == "<EMAIL>",
                "URL/email recognition and post-token punctuation filtering must still work");
        require(tokens[2].sentenceNumber == 2,
                "Sentence boundaries must survive punctuation removal");

        config.replaceUrlsAndEmailsWithMarker = false;
        application.setConfig(config);
        Document document;
        document.tokens.emplace_back(u8"ПРИВІТ", TokenType::Word, 0, 1);
        Dataset dataset;
        dataset.addDocument(document);
        application.runOnDataset(dataset);
        require(application.getDataset().getDocuments()[0].tokens[0].text == u8"привіт",
                "Already tokenized datasets must retain Unicode lowercase support");

        PreprocessingConfig explicitOptions;
        explicitOptions.lowercase = false;
        explicitOptions.trimWhitespace = false;
        explicitOptions.collapseHorizontalWhitespace = false;
        explicitOptions.newlineMode = NewlineMode::NormalizeToLf;
        config.preprocessingConfig = explicitOptions;
        application.setConfig(config);
        require(application.preprocessText(" A\r\n B ") == " A\n B ",
                "Explicit preprocessing options must override legacy flags independently");

        GuiConfig gui;
        gui.inputText = u8"  ПРИВІТ\r\n СВІТ! ";
        gui.strategy = TokenizerStrategyType::Basic;
        gui.applyMorphology = false;
        MainWindow window(gui);
        window.runTokenizationStep();
        const auto& documents = window.getCurrentDataset().getDocuments();
        require(documents.size() == 1 && documents[0].rawText == gui.inputText,
                "GUI must pass the entire document, including newlines, to Application");
        require(documents[0].tokens.size() == 3 && documents[0].tokens[0].text == u8"привіт" &&
                    documents[0].tokens[1].text == u8"світ" &&
                    documents[0].tokens[2].type == TokenType::Punctuation,
                "GUI must use the modular UTF-8 basic tokenizer and preprocessor");

        gui.lowercase = false;
        gui.removePunctuation = true;
        gui.newlineMode = NewlineMode::Preserve;
        gui.trimWhitespace = false;
        gui.normalizeWhitespace = false;
        window.setConfig(gui);
        window.runTokenizationStep();
        const auto& caseTokens = window.getCurrentDataset().getDocuments()[0].tokens;
        require(caseTokens.size() == 2 && caseTokens[0].text == u8"ПРИВІТ",
                "GUI must forward changed case and punctuation settings");

        gui.inputText = "Hello... ?! ! !";
        gui.removePunctuation = false;
        gui.preserveMultiCharPunctuation = true;
        window.setConfig(gui);
        window.runTokenizationStep();
        const auto& grouped = window.getCurrentDataset().getDocuments()[0].tokens;
        require(grouped.size() == 5 && grouped[1].text == "..." && grouped[2].text == "?!" &&
                    grouped[3].text == "!" && grouped[4].text == "!",
                "Grouping must preserve whitespace boundaries");
        gui.preserveMultiCharPunctuation = false;
        window.setConfig(gui);
        window.runTokenizationStep();
        require(window.getCurrentDataset().getDocuments()[0].tokens.size() == 8,
                "GUI must forward disabled punctuation grouping");

        gui.inputText = "Visit https://example.com user@example.com";
        gui.strategy = TokenizerStrategyType::RuleBased;
        window.setConfig(gui);
        window.runTokenizationStep();
        const auto& ruleTokens = window.getCurrentDataset().getDocuments()[0].tokens;
        require(ruleTokens.size() == 3 && ruleTokens[1].text == "https://example.com" &&
                    ruleTokens[2].text == "user@example.com",
                "GUI RuleBased must use the modular strategy through Application");

        gui.strategy = TokenizerStrategyType::Whitespace;
        gui.inputText = "Hello, world!";
        window.setConfig(gui);
        window.runTokenizationStep();
        require(window.getCurrentDataset().getDocuments()[0].tokens.size() == 2,
                "GUI Whitespace must use the selected real strategy");

        gui.strategy = TokenizerStrategyType::RuleBased;
        gui.inputText = u8"The e\u0301 costs 12.50! https://example.com user@example.com";
        gui.stopWordsText = "the costs";
        gui.removePunctuation = true;
        gui.normalization.unicode = UnicodeNormalization::NFC;
        gui.normalization.removeStopWords = true;
        gui.normalization.replaceNumbers = true;
        gui.normalization.replaceUrls = true;
        gui.normalization.replaceEmails = true;
        window.setConfig(gui);
        window.runTokenizationStep();
        const auto& normalizedDocument = window.getCurrentDataset().getDocuments()[0];
        require(normalizedDocument.rawText == gui.inputText && normalizedDocument.tokens.size() == 4 &&
                    normalizedDocument.tokens[0].text == u8"é" && normalizedDocument.tokens[1].text == "<NUM>" &&
                    normalizedDocument.tokens[2].text == "<URL>" && normalizedDocument.tokens[3].text == "<EMAIL>",
                "GUI must forward all normalization options and preserve raw text");

        gui.normalization = {};
        gui.removePunctuation = false;
        gui.inputText = "The price is 12.50!";
        window.setConfig(gui);
        window.runTokenizationStep();
        require(window.getCurrentDataset().getDocuments()[0].tokens.size() == 5,
                "Disabling normalization must restore unfiltered tokenization");

        PipelineConfig normalizationPipeline;
        normalizationPipeline.tokenizerType = TokenizerType::RuleBased;
        NormalizationConfig normalizationOptions;
        normalizationOptions.replaceEmails = true;
        normalizationPipeline.normalizationConfig = normalizationOptions;
        Application normalizedApplication(normalizationPipeline);
        const auto emailOnly = normalizedApplication.tokenizeText("https://example.com user@example.com");
        require(emailOnly[0].text == "https://example.com" && emailOnly[1].text == "<EMAIL>",
                "Application must support independently configured URL/email replacements");
        std::cout << "Application preprocessing checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
