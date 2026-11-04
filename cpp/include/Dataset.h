#pragma once
#include <vector>
#include "Document.h"

class Dataset {
private:
    std::vector<Document> documents;

public:
    Dataset() = default;

    void addDocument(const Document& doc) {
        documents.push_back(doc);
    }

    std::vector<Document>& getDocuments() {
        return documents;
    }

    const std::vector<Document>& getDocuments() const {
        return documents;
    }

    size_t size() const {
        return documents.size();
    }

    bool empty() const {
        return documents.empty();
    }

    void clear() {
        documents.clear();
    }
};