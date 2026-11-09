#pragma once
#include <string>
#include <vector>
#include "Token.h"

struct Document {
    int id = 0;                             // Unique document ID (1, 2, 3...)
    std::string filePath;                   // File path or document name
    std::string rawText;                    // Original raw text
    std::vector<Token> tokens;              // Extracted tokens (output from Role 1 and Role 2)
    
    // Fields populated by Role 3 module (Stage 2 - Vectorization):
    std::vector<int> integerSequence;       // Sequence of numeric token IDs
    std::vector<int> oneHotVector;          // Binary presence vector of size |V|
    std::vector<int> bowVector;             // Bag-of-Words frequency vector of size |V|
    std::vector<double> tfidfVector;        // TF-IDF weighted vector of size |V|
};