#include "../include/Application.h"
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
    std::string processed;
    processed.reserve(rawText.size());

    bool prevSpace = false;
    for (unsigned char ch : rawText) {
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ') {
            if (config.normalizeWhitespace) {
                if (!prevSpace && !processed.empty()) {
                    processed.push_back(' ');
                    prevSpace = true;
                }
            } else {
                processed.push_back(static_cast<char>(ch));
            }
            continue;
        }

        prevSpace = false;
        unsigned char outChar = ch;
        if (config.convertToLowercase && outChar < 128) {
            outChar = static_cast<unsigned char>(std::tolower(outChar));
        }

        if (config.removePunctuation && outChar < 128 && std::ispunct(outChar)) {
            continue;
        }

        processed.push_back(static_cast<char>(outChar));
    }

    if (config.normalizeWhitespace && !processed.empty() && processed.back() == ' ') {
        processed.pop_back();
    }

    return processed;
}

std::vector<Token> Application::applyPostTokenFilters(const std::vector<Token>& rawTokens) const {
    std::vector<Token> filtered;
    filtered.reserve(rawTokens.size());

    size_t newPosition = 0;
    for (const auto& tok : rawTokens) {
        if (config.removePunctuation && tok.type == TokenType::Punctuation) {
            continue;
        }

        Token updated = tok;
        updated.position = newPosition++;

        if (config.convertToLowercase) {
            for (char& c : updated.text) {
                unsigned char uc = static_cast<unsigned char>(c);
                if (uc < 128) {
                    c = static_cast<char>(std::tolower(uc));
                }
            }
        }

        filtered.push_back(std::move(updated));
    }

    return filtered;
}

std::vector<Token> Application::tokenizeBuiltIn(const std::string& text, TokenizerType strategy) const {
    std::vector<Token> tokens;
    size_t pos = 0;
    size_t sentenceNum = 1;
    size_t n = text.size();

    auto classifySingleToken = [](const std::string& s) -> TokenType {
        if (s.empty()) return TokenType::Unknown;
        bool allDigits = true;
        bool allAlpha = true;
        bool allPunct = true;
        for (unsigned char c : s) {
            if (!std::isdigit(c)) allDigits = false;
            if (!std::isalpha(c) && c < 128) allAlpha = false;
            if (!std::ispunct(c)) allPunct = false;
        }
        if (allDigits) return TokenType::Number;
        if (allAlpha) return TokenType::Word;
        if (allPunct && s.size() == 1 &&
            (s[0] == '.' || s[0] == ',' || s[0] == '!' || s[0] == '?' ||
             s[0] == ';' || s[0] == ':' || s[0] == '-' || s[0] == '(' || s[0] == ')')) {
            return TokenType::Punctuation;
        }
        if (s.find('@') != std::string::npos || s.find('#') != std::string::npos ||
            s.find('$') != std::string::npos || s.find("http") == 0 || s.find("C++") != std::string::npos) {
            return TokenType::Special;
        }
        if (allPunct) return TokenType::Punctuation;
        return TokenType::Word;
    };

    if (strategy == TokenizerType::Whitespace) {
        size_t i = 0;
        while (i < n) {
            while (i < n && std::isspace(static_cast<unsigned char>(text[i]))) {
                ++i;
            }
            if (i >= n) break;
            size_t start = i;
            while (i < n && !std::isspace(static_cast<unsigned char>(text[i]))) {
                ++i;
            }
            std::string piece = text.substr(start, i - start);
            TokenType type = classifySingleToken(piece);
            tokens.emplace_back(piece, type, pos++, sentenceNum, -1);
            if (!piece.empty() && (piece.back() == '.' || piece.back() == '!' || piece.back() == '?')) {
                ++sentenceNum;
            }
        }
        return tokens;
    }

    size_t i = 0;
    while (i < n) {
        unsigned char ch = static_cast<unsigned char>(text[i]);
        if (std::isspace(ch)) {
            ++i;
            continue;
        }

        if (strategy == TokenizerType::RuleBased) {
            size_t start = i;
            if (text[i] == '#') {
                ++i;
                while (i < n && (std::isalnum(static_cast<unsigned char>(text[i])) || text[i] == '_')) {
                    ++i;
                }
                if (i > start + 1) {
                    tokens.emplace_back(text.substr(start, i - start), TokenType::Special, pos++, sentenceNum, -1);
                    continue;
                }
                i = start;
            }

            size_t chunkEnd = i;
            while (chunkEnd < n && !std::isspace(static_cast<unsigned char>(text[chunkEnd]))) {
                ++chunkEnd;
            }
            while (chunkEnd > i && (text[chunkEnd - 1] == '.' || text[chunkEnd - 1] == ',' ||
                                    text[chunkEnd - 1] == '!' || text[chunkEnd - 1] == '?')) {
                --chunkEnd;
            }
            std::string candidate = text.substr(i, chunkEnd - i);
            if (candidate.find('@') != std::string::npos ||
                candidate.find("://") != std::string::npos ||
                candidate == "c++" || candidate == "C++") {
                tokens.emplace_back(candidate, TokenType::Special, pos++, sentenceNum, -1);
                i = chunkEnd;
                continue;
            }
        }

        if (std::isalpha(ch) || ch >= 128) {
            size_t start = i;
            while (i < n) {
                unsigned char c = static_cast<unsigned char>(text[i]);
                if (std::isalpha(c) || c >= 128 ||
                    (strategy == TokenizerType::RuleBased && (c == '\'' || c == '-') && i + 1 < n &&
                     (std::isalpha(static_cast<unsigned char>(text[i + 1])) ||
                      static_cast<unsigned char>(text[i + 1]) >= 128))) {
                    ++i;
                } else {
                    break;
                }
            }
            tokens.emplace_back(text.substr(start, i - start), TokenType::Word, pos++, sentenceNum, -1);
            continue;
        }

        if (std::isdigit(ch)) {
            size_t start = i;
            while (i < n && std::isdigit(static_cast<unsigned char>(text[i]))) {
                ++i;
            }
            if (strategy == TokenizerType::RuleBased && i + 1 < n &&
                text[i] == '.' && std::isdigit(static_cast<unsigned char>(text[i + 1]))) {
                ++i;
                while (i < n && std::isdigit(static_cast<unsigned char>(text[i]))) {
                    ++i;
                }
            }
            tokens.emplace_back(text.substr(start, i - start), TokenType::Number, pos++, sentenceNum, -1);
            continue;
        }

        std::string sym(1, text[i]);
        bool isStandardPunct = (text[i] == '.' || text[i] == ',' || text[i] == '!' ||
                                text[i] == '?' || text[i] == ';' || text[i] == ':' ||
                                text[i] == '-' || text[i] == '(' || text[i] == ')' ||
                                text[i] == '"' || text[i] == '\'');
        TokenType symType = isStandardPunct ? TokenType::Punctuation : TokenType::Special;
        tokens.emplace_back(sym, symType, pos++, sentenceNum, -1);

        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            ++sentenceNum;
        }
        ++i;
    }

    return tokens;
}

std::vector<Token> Application::tokenizeText(const std::string& text) const {
    std::string preprocessed = preprocessText(text);
    std::vector<Token> extracted;

    if (config.useExternalTokenizerFactory) {
        auto strategy = TokenizerFactory::create(config.tokenizerType);
        extracted = strategy->tokenize(preprocessed);
        extracted = applyPostTokenFilters(extracted);
    } else {
        extracted = tokenizeBuiltIn(preprocessed, config.tokenizerType);
        extracted = applyPostTokenFilters(extracted);
    }

    if (config.enableMorphologyNormalization ||
        config.replaceNumbersWithMarker ||
        config.replaceUrlsAndEmailsWithMarker) {
        normalizeTokens(extracted);
    }

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
    for (auto& token : tokens) {
        if (config.replaceNumbersWithMarker && token.type == TokenType::Number) {
            token.text = "<NUM>";
            continue;
        }

        if (config.replaceUrlsAndEmailsWithMarker && token.type == TokenType::Special) {
            if (token.text.find('@') != std::string::npos) {
                token.text = "<EMAIL>";
                continue;
            }
            if (token.text.find("://") != std::string::npos || token.text.find("www.") == 0) {
                token.text = "<URL>";
                continue;
            }
        }

        if (config.enableMorphologyNormalization && token.type == TokenType::Word) {
            if (customMorphologyNormalizer) {
                token.text = customMorphologyNormalizer(token.text);
            } else {
                token.text = defaultMorphologyStem(token.text);
            }
        }
    }
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
            if (config.enableMorphologyNormalization ||
                config.replaceNumbersWithMarker ||
                config.replaceUrlsAndEmailsWithMarker) {
                normalizeTokens(doc.tokens);
            }
        }
    }

    return executeVectorizationAndAnalytics();
}

PipelineResult Application::runOnDirectory(const std::string& directoryPath, const std::string& extension) {
    dataset.clear();
    dataset.loadFromDirectory(
        directoryPath,
        extension,
        true,
        [this](const std::string& text) {
            return this->tokenizeText(text);
        }
    );

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
       << (config.useExternalTokenizerFactory ? " (TokenizerFactory)" : " (Built-in Pipeline)") << "\n";
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