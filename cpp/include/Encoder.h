#pragma once
#include <string>
#include "Dataset.h"
#include "Vocabulary.h"

// Abstract base interface for document and dataset vectorization strategies
class Encoder {
public:
    virtual ~Encoder() = default;

    // Encodes a single document using the provided vocabulary
    virtual void encodeDocument(Document& doc, const Vocabulary& vocab) const = 0;

    // Encodes all documents in the dataset
    virtual void encodeDataset(Dataset& dataset, const Vocabulary& vocab) const {
        for (auto& doc : dataset.getDocuments()) {
            encodeDocument(doc, vocab);
        }
    }

    // Returns the name of the encoding strategy
    virtual std::string getName() const = 0;
};