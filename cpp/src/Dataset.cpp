#include "../include/Dataset.h"
#include "../include/Vocabulary.h"
#include "../include/IntegerEncoder.h"
#include "../include/OneHotEncoder.h"
#include "../include/BagOfWordsEncoder.h"
#include "../include/TfIdfEncoder.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

void Dataset::addDocument(const Document& doc) {
    documents.push_back(doc);
}

std::vector<Document>& Dataset::getDocuments() {
    return documents;
}

const std::vector<Document>& Dataset::getDocuments() const {
    return documents;
}

size_t Dataset::size() const {
    return documents.size();
}

bool Dataset::empty() const {
    return documents.empty();
}

void Dataset::clear() {
    documents.clear();
}

std::vector<Token> Dataset::defaultTokenize(const std::string& rawText, bool lowercase) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t n = rawText.size();
    size_t position = 0;
    size_t sentenceNumber = 0;

    while (i < n) {
        unsigned char ch = static_cast<unsigned char>(rawText[i]);

        if (std::isspace(ch)) {
            ++i;
            continue;
        }

        // Words (alphabetic sequences)
        if (std::isalpha(ch)) {
            std::string word;
            while (i < n && std::isalpha(static_cast<unsigned char>(rawText[i]))) {
                char c = rawText[i];
                if (lowercase) {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                word += c;
                ++i;
            }
            tokens.emplace_back(word, TokenType::Word, position++, sentenceNumber, -1);
            continue;
        }

        // Numbers (integer and decimal sequences)
        if (std::isdigit(ch)) {
            std::string num;
            while (i < n && (std::isdigit(static_cast<unsigned char>(rawText[i])) ||
                             (rawText[i] == '.' && i + 1 < n && std::isdigit(static_cast<unsigned char>(rawText[i + 1]))))) {
                num += rawText[i];
                ++i;
            }
            tokens.emplace_back(num, TokenType::Number, position++, sentenceNumber, -1);
            continue;
        }

        // Punctuation vs Special characters
        std::string sym(1, rawText[i]);
        bool isPunct = (rawText[i] == '.' || rawText[i] == ',' || rawText[i] == '!' ||
                        rawText[i] == '?' || rawText[i] == ':' || rawText[i] == ';' ||
                        rawText[i] == '(' || rawText[i] == ')' || rawText[i] == '[' ||
                        rawText[i] == ']' || rawText[i] == '{' || rawText[i] == '}' ||
                        rawText[i] == '"' || rawText[i] == '\'' || rawText[i] == '-');

        TokenType type = isPunct ? TokenType::Punctuation : TokenType::Special;
        tokens.emplace_back(sym, type, position++, sentenceNumber, -1);

        if (rawText[i] == '.' || rawText[i] == '!' || rawText[i] == '?') {
            ++sentenceNumber;
        }
        ++i;
    }

    return tokens;
}

void Dataset::tokenizeAll(TokenizerFunc customTokenizer, bool lowercase) {
    for (auto& doc : documents) {
        if (customTokenizer) {
            doc.tokens = customTokenizer(doc.rawText);
        } else {
            doc.tokens = defaultTokenize(doc.rawText, lowercase);
        }
    }
}

bool Dataset::loadDocumentFromFile(const std::string& filePath,
                                   bool autoTokenize,
                                   TokenizerFunc customTokenizer) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    Document doc;
    doc.id = static_cast<int>(documents.size()) + 1;
    doc.filePath = filePath;
    doc.rawText = buffer.str();

    if (autoTokenize) {
        if (customTokenizer) {
            doc.tokens = customTokenizer(doc.rawText);
        } else {
            doc.tokens = defaultTokenize(doc.rawText, true);
        }
    }

    documents.push_back(std::move(doc));
    return true;
}

size_t Dataset::loadFromDirectory(const std::string& directoryPath,
                                  const std::string& extension,
                                  bool autoTokenize,
                                  TokenizerFunc customTokenizer) {
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        return 0;
    }

    std::vector<fs::path> filePaths;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            if (extension.empty() || entry.path().extension().string() == extension) {
                filePaths.push_back(entry.path());
            }
        }
    }

    // Sort paths alphabetically for deterministic document numbering
    std::sort(filePaths.begin(), filePaths.end());

    size_t loadedCount = 0;
    for (const auto& path : filePaths) {
        if (loadDocumentFromFile(path.string(), autoTokenize, customTokenizer)) {
            ++loadedCount;
        }
    }

    return loadedCount;
}

void Dataset::applyEncoder(const Encoder& encoder, const Vocabulary& vocab) {
    encoder.encodeDataset(*this, vocab);
}

void Dataset::vectorizeAll(const Vocabulary& vocab) {
    IntegerEncoder intEncoder;
    OneHotEncoder oneHotEncoder;
    BagOfWordsEncoder bowEncoder;
    TfIdfEncoder tfidfEncoder(false);

    vectorizeAll(vocab, intEncoder, oneHotEncoder, bowEncoder, tfidfEncoder);
}

void Dataset::vectorizeAll(const Vocabulary& vocab,
                           const IntegerEncoder& intEncoder,
                           const OneHotEncoder& oneHotEncoder,
                           const BagOfWordsEncoder& bowEncoder,
                           const TfIdfEncoder& tfidfEncoder) {
    vocab.assignIdsToDataset(*this);
    intEncoder.encodeDataset(*this, vocab);
    oneHotEncoder.encodeDataset(*this, vocab);
    bowEncoder.encodeDataset(*this, vocab);
    tfidfEncoder.encodeDataset(*this, vocab);
}

std::vector<Dataset> Dataset::splitIntoBatches(size_t batchSize) const {
    std::vector<Dataset> batches;
    if (batchSize == 0 || documents.empty()) {
        return batches;
    }

    for (size_t i = 0; i < documents.size(); i += batchSize) {
        Dataset batch;
        size_t end = std::min(i + batchSize, documents.size());
        for (size_t j = i; j < end; ++j) {
            batch.addDocument(documents[j]);
        }
        batches.push_back(std::move(batch));
    }

    return batches;
}

void Dataset::processInBatches(size_t batchSize,
                               const Vocabulary& vocab,
                               const std::function<void(Dataset&, size_t)>& batchCallback) {
    if (batchSize == 0 || documents.empty()) {
        return;
    }

    // Precompute corpus-wide TF-IDF and all document representations
    vectorizeAll(vocab);

    size_t batchIndex = 0;
    for (size_t i = 0; i < documents.size(); i += batchSize) {
        Dataset batch;
        size_t end = std::min(i + batchSize, documents.size());
        for (size_t j = i; j < end; ++j) {
            batch.addDocument(documents[j]);
        }

        if (batchCallback) {
            batchCallback(batch, batchIndex);
        }

        // Write back any changes made inside the batch callback
        const auto& updatedDocs = batch.getDocuments();
        for (size_t j = 0; j < updatedDocs.size(); ++j) {
            documents[i + j] = updatedDocs[j];
        }

        ++batchIndex;
    }
}