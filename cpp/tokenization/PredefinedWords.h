#pragma once

#include "../include/Token.h"
#include <vector>

namespace PredefinedWords {
struct Entry {
    std::u32string text;
    TokenType type;
};

// Edit this dictionary to add exceptions, then rebuild.
// Matching ignores Unicode letter case and preserves the input spelling.
inline const std::vector<Entry>& dictionary() {
    static const std::vector<Entry> entries = {
        {U"C++", TokenType::Special},
        {U"C++/CLI", TokenType::Special},
        {U"C#", TokenType::Special},
        {U".NET", TokenType::Special},
    };
    return entries;
}
}
