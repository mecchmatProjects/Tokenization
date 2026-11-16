#pragma once
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include "Token.h"
#include "Document.h"
#include "Dataset.h"
#include "Vocabulary.h"
#include "IntegerEncoder.h"
#include "OneHotEncoder.h"
#include "BagOfWordsEncoder.h"
#include "TfIdfEncoder.h"
#include "StatisticsAnalyzer.h"
#include "../tokenization/TokenizerFactory.h"

// Configuration parameters for the end-to-end NLP processing pipeline
struct PipelineConfig {
    // Role 1: Tokenization & Preprocessing settings
    TokenizerType tokenizerType = TokenizerType::WordPunctuation;
    bool useExternalTokenizerFactory = true; // true: use TokenizerFactory::create(), false: built-in tokenizer
    bool normalizeWhitespace = true;
    bool convertToLowercase = true;
    bool removePunctuation = false;

    // Role 2: Morphology & Word Normalization settings
    bool enableMorphologyNormalization = true;
    bool replaceNumbersWithMarker = false;
    bool replaceUrlsAndEmailsWithMarker = false;

    // Role 3: Vocabulary, Vectorization & Analytics settings
    int minFrequency = 1;
    size_t maxVocabularySize = 0; // 0 means unlimited
    bool addBoundaryTokens = false;
    size_t maxLength = 0;         // 0 means dynamic sequence length
    bool postPadding = true;
    bool postTruncation = true;
    bool smoothIdf = false;
    size_t topNFrequencies = 10;
};

// Aggregated output produced by a complete pipeline run
struct PipelineResult {
    CorpusStatistics statistics;
    std::vector<TokenFrequencyInfo> tokenFrequencies;
    std::vector<TokenTypeStats> typeDistribution;
    size_t vocabularySize = 0;
    size_t paddingTokensCount = 0;
    double paddingRatio = 0.0;
};

// Central controller integrating preprocessing, tokenization, morphology, vectorization, and analytics
class Application {
public:
    using MorphologyFunc = std::function<std::string(const std::string&)>;

private:
    PipelineConfig config;
    Dataset dataset;
    Vocabulary vocabulary;
    CorpusStatistics lastStatistics;
    MorphologyFunc customMorphologyNormalizer;

    // Built-in fallback tokenization strategies for real text processing
    std::vector<Token> tokenizeBuiltIn(const std::string& text, TokenizerType strategy) const;

    // Applies post-tokenization filters (case folding and punctuation removal)
    std::vector<Token> applyPostTokenFilters(const std::vector<Token>& rawTokens) const;

    // Runs vocabulary building, all encoders, and statistical analysis on the current dataset
    PipelineResult executeVectorizationAndAnalytics();

public:
    Application();
    explicit Application(const PipelineConfig& initialConfig);

    // Configuration management
    void setConfig(const PipelineConfig& newConfig);
    const PipelineConfig& getConfig() const;

    // Hook for integrating external Morphology / Word processing module (Role 2)
    void setMorphologyNormalizer(MorphologyFunc normalizer);

    // Stage 1: Text preprocessing (whitespace normalization, case folding, punctuation stripping)
    std::string preprocessText(const std::string& rawText) const;

    // Stage 2: Tokenization via Role 1 TokenizerFactory or built-in strategy
    std::vector<Token> tokenizeText(const std::string& text) const;

    // Stage 3: Morphological and lexical normalization of extracted tokens
    void normalizeTokens(std::vector<Token>& tokens) const;

    // Default rule-based morphological stemmer/normalizer for word tokens
    static std::string defaultMorphologyStem(const std::string& word);

    // Full pipeline execution entry points
    PipelineResult runOnText(const std::string& rawText, const std::string& docName = "input_document");
    PipelineResult runOnTexts(const std::vector<std::string>& rawTexts);
    PipelineResult runOnDataset(const Dataset& inputDataset);
    PipelineResult runOnDirectory(const std::string& directoryPath, const std::string& extension = ".txt");

    // Accessors for pipeline state
    const Dataset& getDataset() const;
    Dataset& getDataset();
    const Vocabulary& getVocabulary() const;
    const CorpusStatistics& getStatistics() const;

    // Utility helper to convert TokenizerType enum to readable string
    static std::string tokenizerTypeToString(TokenizerType type);

    // Prints a formatted summary of the pipeline execution to an output stream
    void printPipelineSummary(std::ostream& os = std::cout) const;
};