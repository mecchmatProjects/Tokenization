#include <cassert>
#include <chrono>
#include <clocale>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lemmatizer.h"
#include "preprocessor.h"
#include "tokenizer.h"

// Helper to read raw text from a file
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    assert(file.is_open() && "Failed to open test file");
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper to count tokens of a specific type
size_t countByType(const std::vector<Token>& tokens, TokenType type) {
    size_t count = 0;
    for (const auto& t : tokens) {
        if (t.getType() == type) {
            ++count;
        }
    }
    return count;
}

// Run full pipeline on a file and return resulting tokens
std::vector<Token> runPipelineOnFile(const std::string& path, const Lemmatizer& lemmatizer) {
    std::string raw = readFile(path);
    std::string cleaned = Preprocessor::process(raw, false, false);
    std::vector<Token> tokens = Tokenizer::tokenize(cleaned);
    if (!tokens.empty()) {
        lemmatizer.lemmatizeTokens(tokens);
    }
    return tokens;
}

// Test empty.txt
void testEmptyFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/empty.txt", uk);
    assert(tokens.empty());
    std::cout << "[PASS] empty.txt\n";
}

// Test one_sentence.txt
void testOneSentenceFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/one_sentence.txt", uk);
    assert(countByType(tokens, TokenType::WORD) == 6);
    assert(tokens.back().getSentenceNumber() == 1);
    std::cout << "[PASS] one_sentence.txt\n";
}

// Test complex_words.txt (numbers, hyphens, uppercase acronyms)
void testComplexWordsFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/complex_words.txt", uk);
    assert(countByType(tokens, TokenType::WORD) > 0);
    assert(countByType(tokens, TokenType::NUMBER) >= 1);
    std::cout << "[PASS] complex_words.txt\n";
}

// Test dialogues_and_punctuation.txt
void testDialoguesFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/dialogues_and_punctuation.txt", uk);
    assert(countByType(tokens, TokenType::PUNCTUATION) > 5);
    assert(tokens.back().getSentenceNumber() >= 5);
    std::cout << "[PASS] dialogues_and_punctuation.txt\n";
}

// Test english_test.txt with English dictionary
void testEnglishFile(const Lemmatizer& en) {
    auto tokens = runPipelineOnFile("datasets/technical/english_test.txt", en);
    assert(countByType(tokens, TokenType::WORD) == 21);
    assert(tokens.back().getSentenceNumber() == 2);
    std::cout << "[PASS] english_test.txt\n";
}

// Test mixed_unicode.txt (negative floats, apostrophes, mixed languages)
void testMixedUnicodeFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/mixed_unicode.txt", uk);
    assert(countByType(tokens, TokenType::NUMBER) >= 1);
    assert(tokens.back().getSentenceNumber() == 2);
    std::cout << "[PASS] mixed_unicode.txt\n";
}

// Test repetitions.txt
void testRepetitionsFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/repetitions.txt", uk);
    assert(tokens.back().getSentenceNumber() == 4);
    std::cout << "[PASS] repetitions.txt\n";
}

// Stress-test on a generated large document
void testLargeDocument(const Lemmatizer& uk) {
    const std::string baseParagraph = "Студенти написали чудовий код. Програма працює дуже швидко! ";
    const int repetitions = 2000;

    std::string largeText;
    largeText.reserve(baseParagraph.size() * repetitions);
    for (int i = 0; i < repetitions; ++i) {
        largeText += baseParagraph;
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::string cleaned = Preprocessor::process(largeText, false, false);
    std::vector<Token> tokens = Tokenizer::tokenize(cleaned);
    uk.lemmatizeTokens(tokens);

    auto end = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    assert(countByType(tokens, TokenType::WORD) == static_cast<size_t>(repetitions * 8));
    assert(tokens.back().getSentenceNumber() == repetitions * 2);

    std::cout << "[PASS] Large document (" << largeText.size() << " bytes) in "
              << durationMs << " ms\n";
}

// Test abbreviations.txt (abbreviations, initials, year, parentheses)
void testAbbreviationsFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/abbreviations.txt", uk);
    assert(countByType(tokens, TokenType::WORD) == 23);
    assert(countByType(tokens, TokenType::NUMBER) == 1);
    assert(countByType(tokens, TokenType::PUNCTUATION) == 14);
    std::cout << "[PASS] abbreviations.txt\n";
}

// Test dirty_text.txt (extra spaces, tabs, multiple newlines, ellipsis)
void testDirtyTextFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/dirty_text.txt", uk);
    assert(countByType(tokens, TokenType::WORD) == 14);
    assert(countByType(tokens, TokenType::PUNCTUATION) == 2);
    assert(tokens.back().getSentenceNumber() == 2);
    std::cout << "[PASS] dirty_text.txt\n";
}

// Test long_sentece.txt (single very long sentence with commas and hyphenated word)
void testLongSentenceFile(const Lemmatizer& uk) {
    auto tokens = runPipelineOnFile("datasets/technical/long_sentence.txt", uk);
    assert(countByType(tokens, TokenType::WORD) == 82);
    assert(tokens.back().getSentenceNumber() == 1);
    std::cout << "[PASS] long_sentence.txt\n";
}

int main() {
    std::setlocale(LC_ALL, "uk_UA.UTF-8");

    Lemmatizer ukLemmatizer;
    assert(ukLemmatizer.loadDictionary("datasets/dictionaries/uk_dict.txt"));

    Lemmatizer enLemmatizer;
    assert(enLemmatizer.loadDictionary("datasets/dictionaries/en_dict.txt"));

    testEmptyFile(ukLemmatizer);
    testOneSentenceFile(ukLemmatizer);
    testComplexWordsFile(ukLemmatizer);
    testDialoguesFile(ukLemmatizer);
    testEnglishFile(enLemmatizer);
    testMixedUnicodeFile(ukLemmatizer);
    testRepetitionsFile(ukLemmatizer);
    testAbbreviationsFile(ukLemmatizer);
    testDirtyTextFile(ukLemmatizer);
    testLongSentenceFile(ukLemmatizer);
    testLargeDocument(ukLemmatizer);

    std::cout << "All technical & pipeline tests passed!\n";
}