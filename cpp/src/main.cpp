#include <iostream>
#include <iomanip>
#include "../include/Application.h"

int main() {
    std::cout << "====================================================================\n";
    std::cout << " SCENARIO 1: Integration with External TokenizerFactory (Role 1 PR) \n";
    std::cout << "====================================================================\n";
    {
        PipelineConfig cfg;
        cfg.tokenizerType = TokenizerType::WordPunctuation;
        cfg.useExternalTokenizerFactory = true; // Calls TokenizerFactory::create()
        cfg.convertToLowercase = true;
        cfg.removePunctuation = false;
        cfg.enableMorphologyNormalization = true;
        cfg.maxLength = 6; // Pad sequence to length 6

        Application app(cfg);
        PipelineResult res = app.runOnText("Sample input text for factory integration", "factory_doc.txt");

        app.printPipelineSummary(std::cout);
        std::cout << "Padding tokens added: " << res.paddingTokensCount
                  << " (" << std::fixed << std::setprecision(2) << (res.paddingRatio * 100.0) << "%)\n\n";
    }

    std::cout << "====================================================================\n";
    std::cout << " SCENARIO 2: Full End-to-End Pipeline (Preprocess + Morph + Vectors)\n";
    std::cout << "====================================================================\n";
    {
        PipelineConfig cfg;
        cfg.tokenizerType = TokenizerType::RuleBased;
        cfg.useExternalTokenizerFactory = false; // Built-in pipeline for real multi-doc corpus
        cfg.convertToLowercase = true;
        cfg.removePunctuation = false;
        cfg.enableMorphologyNormalization = true;
        cfg.minFrequency = 1;
        cfg.maxLength = 8;

        Application app(cfg);
        std::vector<std::string> corpus = {
            "Machine learning models are learning from data!",
            "Natural language processing systems process text data.",
            "Contact nlp@university.edu for #machinelearning in C++ 42."
        };

        PipelineResult res = app.runOnTexts(corpus);
        app.printPipelineSummary(std::cout);

        StatisticsAnalyzer::printTokenFrequencyTable(res.tokenFrequencies, "Top Token Frequencies");
        std::cout << "\n";
        StatisticsAnalyzer::printTokenTypeDistribution(res.typeDistribution, "Token Type Distribution");
        std::cout << "\n";
    }

    std::cout << "====================================================================\n";
    std::cout << " SCENARIO 3: Strategy Comparison via Application Controller         \n";
    std::cout << "====================================================================\n";
    {
        const std::string sample = "Email user@test.com uses C++ & #NLP, price=100!";
        const std::vector<TokenizerType> strategies = {
            TokenizerType::Whitespace,
            TokenizerType::WordPunctuation,
            TokenizerType::RuleBased
        };

        for (TokenizerType strat : strategies) {
            PipelineConfig cfg;
            cfg.tokenizerType = strat;
            cfg.useExternalTokenizerFactory = false;
            cfg.convertToLowercase = true;
            cfg.enableMorphologyNormalization = true;

            Application app(cfg);
            PipelineResult res = app.runOnText(sample, "strategy_test.txt");

            std::cout << "Strategy [" << Application::tokenizerTypeToString(strat) << "]: "
                      << "totalTokens=" << res.statistics.totalTokens
                      << ", uniqueTokens=" << res.statistics.uniqueTokens
                      << ", vocabSize=" << res.vocabularySize << "\n  Tokens: ";
            const auto& tokens = app.getDataset().getDocuments().front().tokens;
            for (size_t i = 0; i < tokens.size(); ++i) {
                std::cout << "\"" << tokens[i].text << "\"(" << tokenTypeToString(tokens[i].type) << ")";
                if (i + 1 < tokens.size()) std::cout << ", ";
            }
            std::cout << "\n\n";
        }
    }

    return 0;
}