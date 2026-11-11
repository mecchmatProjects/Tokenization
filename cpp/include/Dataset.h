#pragma once
#include <vector>
#include <string>
#include <functional>
#include "Document.h"

class Vocabulary;
class Encoder;
class IntegerEncoder;
class OneHotEncoder;
class BagOfWordsEncoder;
class TfIdfEncoder;

class Dataset {
private:
    std::vector<Document> documents;

public:
    using TokenizerFunc = std::function<std::vector<Token>(const std::string&)>;

    Dataset() = default;

    // Basic document management
    void addDocument(const Document& doc);
    std::vector<Document>& getDocuments();
    const std::vector<Document>& getDocuments() const;
    size_t size() const;
    bool empty() const;
    void clear();

    // File and directory loading (Batch I/O)
    bool loadDocumentFromFile(const std::string& filePath,
                              bool autoTokenize = true,
                              TokenizerFunc customTokenizer = nullptr);

    size_t loadFromDirectory(const std::string& directoryPath,
                             const std::string& extension = ".txt",
                             bool autoTokenize = true,
                             TokenizerFunc customTokenizer = nullptr);

    // Tokenization helpers
    static std::vector<Token> defaultTokenize(const std::string& rawText, bool lowercase = true);
    void tokenizeAll(TokenizerFunc customTokenizer = nullptr, bool lowercase = true);

    // Unified vectorization pipeline
    void applyEncoder(const Encoder& encoder, const Vocabulary& vocab);

    void vectorizeAll(const Vocabulary& vocab);

    void vectorizeAll(const Vocabulary& vocab,
                      const IntegerEncoder& intEncoder,
                      const OneHotEncoder& oneHotEncoder,
                      const BagOfWordsEncoder& bowEncoder,
                      const TfIdfEncoder& tfidfEncoder);

    // Batch processing (mini-batch splitting and iteration)
    std::vector<Dataset> splitIntoBatches(size_t batchSize) const;

    void processInBatches(size_t batchSize,
                          const Vocabulary& vocab,
                          const std::function<void(Dataset&, size_t)>& batchCallback = nullptr);
};