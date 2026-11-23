#include "../include/Application.h"
#include "preprocessing/TextPreprocessor.h"
#include <cctype>
#include <sstream>
#include <iomanip>
#include <algorithm>

Application::Application() = default;

Application::Application(const PipelineConfig& initialConfig)
    : config(initialConfig) {}

void Application::setConfig(const PipelineConfig& newConfig) {
    config = newConfig;
}

const PipelineConfig& Application::getConfig() const {
    return config;
}

void Application::setMorphologyNormalizer(MorphologyFunc normalizer) {
    customMorphologyNormalizer = std::move(normalizer);
}

std::string Application::tokenizerTypeToString(TokenizerType type) {
    switch (type) {
        case TokenizerType::Whitespace:      return "Whitespace";
        case TokenizerType::WordPunctuation: return "WordPunctuation";
        case TokenizerType::RuleBased:       return "RuleBased";
        default:                             return "Unknown";
    }
}

std::string Application::preprocessText(const std::string& rawText) const {
    if (config.preprocessingConfig) {
        return TextPreprocessor(*config.preprocessingConfig).process(rawText);
    }
    PreprocessingConfig options;
    options.collapseHorizontalWhitespace = config.normalizeWhitespace;
    options.trimWhitespace = config.normalizeWhitespace;
    options.lowercase = config.convertToLowercase;
    options.newlineMode = config.normalizeWhitespace
        ? NewlineMode::ReplaceWithSpace : NewlineMode::Preserve;
    return TextPreprocessor(options).process(rawText);
}

std::vector<Token> Application::applyPostTokenFilters(const std::vector<Token>& rawTokens,
                                                     bool normalizeCase) const {
    std::vector<Token> filtered;
    filtered.reserve(rawTokens.size());

    PreprocessingConfig caseOptions;
    caseOptions.collapseHorizontalWhitespace = false;
    caseOptions.trimWhitespace = false;
    caseOptions.newlineMode = NewlineMode::Preserve;
    caseOptions.lowercase = config.preprocessingConfig
        ? config.preprocessingConfig->lowercase : config.convertToLowercase;
    const TextPreprocessor casePreprocessor(caseOptions);

    for (const auto& tok : rawTokens) {
        Token updated = tok;

        if (normalizeCase && caseOptions.lowercase) {
            updated.text = casePreprocessor.process(updated.text);
            if (updated.text != tok.text) updated.tokenId = -1;
        }

        filtered.push_back(std::move(updated));
    }

    return filtered;
}

std::vector<Token> Application::tokenizeText(const std::string& text) const {
    std::string preprocessed = preprocessText(text);
    auto strategy = TokenizerFactory::create(config.tokenizerType);
    std::vector<Token> extracted = strategy->tokenize(preprocessed);

    if (config.preserveMultiCharPunctuation) {
        std::vector<Token> grouped;
        size_t cursor = 0;
        size_t previousEnd = std::string::npos;
        for (const auto& token : extracted) {
            const size_t start = preprocessed.find(token.text, cursor);
            const bool adjacent = start != std::string::npos && start == previousEnd;
            if (adjacent && token.type == TokenType::Punctuation && !token.text.empty() &&
                !grouped.empty() && grouped.back().type == TokenType::Punctuation &&
                !grouped.back().text.empty() &&
                (grouped.back().text.back() == token.text.front() ||
                 (grouped.back().text.front() == '?' && token.text == "!"))) {
                grouped.back().text += token.text;
            } else {
                grouped.push_back(token);
            }
            previousEnd = start == std::string::npos ? start : start + token.text.size();
            if (start != std::string::npos) cursor = previousEnd;
        }
        extracted = std::move(grouped);
    }
    normalizeTokens(extracted);

    return extracted;
}

std::string Application::defaultMorphologyStem(const std::string& word) {
    if (word.size() <= 3) {
        return word;
    }

    static const std::vector<std::string> suffixes = {
        "ization", "ational", "fulness", "ousness", "iveness",
        "ments", "ment", "ings", "ing", "edly", "ed", "ies", "es", "s"
    };

    for (const auto& suffix : suffixes) {
        if (word.size() > suffix.size() + 2 &&
            word.compare(word.size() - suffix.size(), suffix.size(), suffix) == 0) {
            if (suffix == "ies") {
                return word.substr(0, word.size() - 3) + "y";
            }
            if (suffix == "s" && word.size() >= 2 && word[word.size() - 2] == 's') {
                continue;
            }
            if (suffix == "s" && word == "is") {
                return word;
            }
            return word.substr(0, word.size() - suffix.size());
        }
    }

    return word;
}

void Application::normalizeTokens(std::vector<Token>& tokens) const {
    NormalizationConfig options;
    options.removePunctuation = config.removePunctuation;
    options.replaceNumbers = config.replaceNumbersWithMarker;
    options.replaceUrls = config.replaceUrlsAndEmailsWithMarker;
    options.replaceEmails = config.replaceUrlsAndEmailsWithMarker;
    if (config.normalizationConfig) options = *config.normalizationConfig;
    auto normalized = Normalizer().normalize(tokens, options, config.stopWords);
    // Morphology remains a separate operation after lexical normalization.
    if (config.enableMorphologyNormalization) {
        for (auto& token : normalized) {
            if (token.type != TokenType::Word) continue;
            const auto previous = token.text;
            token.text = customMorphologyNormalizer
                ? customMorphologyNormalizer(token.text) : defaultMorphologyStem(token.text);
            if (token.text != previous) token.tokenId = -1;
        }
    }
    tokens = std::move(normalized);
}

PipelineResult Application::executeVectorizationAndAnalytics() {
    vocabulary.buildFromDataset(dataset, config.minFrequency, config.maxVocabularySize);
    vocabulary.assignIdsToDataset(dataset);

    IntegerEncoder intEncoder(config.addBoundaryTokens, config.maxLength);
    intEncoder.setPostPadding(config.postPadding);
    intEncoder.setPostTruncation(config.postTruncation);
    intEncoder.encodeDataset(dataset, vocabulary);

    OneHotEncoder oneHotEncoder;
    oneHotEncoder.encodeDataset(dataset, vocabulary);

    BagOfWordsEncoder bowEncoder;
    bowEncoder.encodeDataset(dataset, vocabulary);

    TfIdfEncoder tfidfEncoder(config.smoothIdf);
    tfidfEncoder.encodeDataset(dataset, vocabulary);

    lastStatistics = StatisticsAnalyzer::analyze(dataset, vocabulary);

    PipelineResult result;
    result.statistics = lastStatistics;
    result.tokenFrequencies = StatisticsAnalyzer::getTokenFrequencyTable(
        dataset, &vocabulary, config.topNFrequencies);
    result.typeDistribution = StatisticsAnalyzer::getTokenTypeDistribution(dataset);
    result.vocabularySize = vocabulary.size();
    result.paddingTokensCount = IntegerEncoder::countDatasetPaddingTokens(dataset);
    result.paddingRatio = IntegerEncoder::calculateDatasetPaddingRatio(dataset);

    return result;
}

PipelineResult Application::runOnText(const std::string& rawText, const std::string& docName) {
    dataset.clear();

    Document doc;
    doc.id = 1;
    doc.filePath = docName;
    doc.rawText = rawText;
    doc.tokens = tokenizeText(rawText);

    dataset.addDocument(doc);
    return executeVectorizationAndAnalytics();
}

PipelineResult Application::runOnTexts(const std::vector<std::string>& rawTexts) {
    dataset.clear();

    int nextDocId = 1;
    for (const auto& text : rawTexts) {
        Document doc;
        doc.id = nextDocId;
        doc.filePath = "doc_" + std::to_string(nextDocId) + ".txt";
        doc.rawText = text;
        doc.tokens = tokenizeText(text);
        dataset.addDocument(doc);
        ++nextDocId;
    }

    return executeVectorizationAndAnalytics();
}

PipelineResult Application::runOnDataset(const Dataset& inputDataset) {
    dataset = inputDataset;

    for (auto& doc : dataset.getDocuments()) {
        if (!doc.rawText.empty() && doc.tokens.empty()) {
            doc.tokens = tokenizeText(doc.rawText);
        } else if (!doc.tokens.empty()) {
            doc.tokens = applyPostTokenFilters(doc.tokens);
            normalizeTokens(doc.tokens);
        }
    }

    return executeVectorizationAndAnalytics();
}

PipelineResult Application::runOnDirectory(const std::string& directoryPath, const std::string& extension) {
    Dataset loaded;
    loaded.loadFromDirectory(
        directoryPath,
        extension,
        true,
        [this](const std::string& text) {
            return this->tokenizeText(text);
        },
        config.textEncoding
    );
    dataset = std::move(loaded);
    return executeVectorizationAndAnalytics();
}

const Dataset& Application::getDataset() const {
    return dataset;
}

Dataset& Application::getDataset() {
    return dataset;
}

const Vocabulary& Application::getVocabulary() const {
    return vocabulary;
}

const CorpusStatistics& Application::getStatistics() const {
    return lastStatistics;
}

void Application::printPipelineSummary(std::ostream& os) const {
    os << "=== Pipeline Execution Summary ===\n";
    os << "Tokenizer strategy:         " << tokenizerTypeToString(config.tokenizerType)
       << " (TokenizerFactory)\n";
    os << "Lowercase / Strip punct:    " << (config.convertToLowercase ? "ON" : "OFF")
       << " / " << (config.removePunctuation ? "ON" : "OFF") << "\n";
    os << "Morphology normalization:   " << (config.enableMorphologyNormalization ? "ON" : "OFF") << "\n";
    os << "Documents processed:        " << lastStatistics.documentCount << "\n";
    os << "Total tokens (T):           " << lastStatistics.totalTokens << "\n";
    os << "Unique corpus tokens:       " << lastStatistics.uniqueTokens << "\n";
    os << "Vocabulary size (with sp.): " << vocabulary.size() << "\n";
    os << "Unique unknown (<UNK>):     " << lastStatistics.uniqueUnknownTokens
       << " (" << std::fixed << std::setprecision(2) << (lastStatistics.unknownTokenRatio * 100.0) << "%)\n";

    for (const auto& doc : dataset.getDocuments()) {
        os << "  [Doc #" << doc.id << " \"" << doc.filePath << "\"] tokens=" << doc.tokens.size()
           << " | tokens: ";
        for (size_t i = 0; i < doc.tokens.size(); ++i) {
            os << doc.tokens[i].text << "(" << tokenTypeToString(doc.tokens[i].type)
               << ", id=" << doc.tokens[i].tokenId << ")";
            if (i + 1 < doc.tokens.size()) os << ", ";
        }
        os << "\n    Integer seq: [";
        for (size_t i = 0; i < doc.integerSequence.size(); ++i) {
            os << doc.integerSequence[i];
            if (i + 1 < doc.integerSequence.size()) os << ", ";
        }
        os << "]\n";
    }
    os << "\n";
}
