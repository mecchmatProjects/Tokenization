#pragma once
#include "Dataset.h"

class MockData {
public:
    // Creates a sample dataset of 3 documents simulating output from Role 1 and Role 2
    static Dataset createSampleDataset() {
        Dataset dataset;

        // Document 1 (based on section 2 of the specification)
        Document doc1;
        doc1.id = 1;
        doc1.filePath = "dataset/doc1.txt";
        doc1.rawText = "machine learning is amazing! i have 2 projects.";
        doc1.tokens = {
            Token("machine", TokenType::Word, 0, 1),
            Token("learning", TokenType::Word, 1, 1),
            Token("is", TokenType::Word, 2, 1),
            Token("amazing", TokenType::Word, 3, 1),
            Token("!", TokenType::Punctuation, 4, 1),
            Token("i", TokenType::Word, 5, 2),
            Token("have", TokenType::Word, 6, 2),
            Token("2", TokenType::Number, 7, 2),
            Token("projects", TokenType::Word, 8, 2),
            Token(".", TokenType::Punctuation, 9, 2)
        };

        // Document 2 (contains repeated words "machine", "learning" and special characters)
        Document doc2;
        doc2.id = 2;
        doc2.filePath = "dataset/doc2.txt";
        doc2.rawText = "machine learning algorithm and data model = 100$";
        doc2.tokens = {
            Token("machine", TokenType::Word, 0, 1),
            Token("learning", TokenType::Word, 1, 1),
            Token("algorithm", TokenType::Word, 2, 1),
            Token("and", TokenType::Word, 3, 1),
            Token("data", TokenType::Word, 4, 1),
            Token("model", TokenType::Word, 5, 1),
            Token("=", TokenType::Special, 6, 1),
            Token("100", TokenType::Number, 7, 1),
            Token("$", TokenType::Special, 8, 1)
        };

        // Document 3 (repeats key terms to test frequency filtering on Day 4)
        Document doc3;
        doc3.id = 3;
        doc3.filePath = "dataset/doc3.txt";
        doc3.rawText = "machine data is fast.";
        doc3.tokens = {
            Token("machine", TokenType::Word, 0, 1),
            Token("data", TokenType::Word, 1, 1),
            Token("is", TokenType::Word, 2, 1),
            Token("fast", TokenType::Word, 3, 1),
            Token(".", TokenType::Punctuation, 4, 1)
        };

        dataset.addDocument(doc1);
        dataset.addDocument(doc2);
        dataset.addDocument(doc3);

        return dataset;
    }
};