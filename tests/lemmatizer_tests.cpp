#include <iostream>
#include <cassert>
#include <clocale>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "lemmatizer.h"
#include "tokenizer.h"



void test_empty_lemmatizer() {
    Lemmatizer lemmatizer;

    assert(lemmatizer.getDictionarySize() == 0);
    assert(lemmatizer.lemmatizeWord("") == "");
    assert(lemmatizer.lemmatizeWord("будинок") == "будинок");

    std::cout << "test_empty_lemetizer passed!\n";
}



void test_add_rule_and_lemmatize_word() {
    Lemmatizer lemmatizer;
    lemmatizer.addRule("будинку", "будинок");
    lemmatizer.addRule("Будинком", "Будинок");
    lemmatizer.addRule("йшли", "йти");

    assert(lemmatizer.getDictionarySize() == 3);

    // Known word forms (including uppercase input)
    assert(lemmatizer.lemmatizeWord("будинку") == "будинок");
    assert(lemmatizer.lemmatizeWord("БУДИНКОМ") == "будинок");
    assert(lemmatizer.lemmatizeWord("Йшли") == "йти");

    // Unknown word should return its normalized form unchanged
    assert(lemmatizer.lemmatizeWord("Сонце") == "сонце");

    // Empty rules should be ignored
    lemmatizer.addRule("", "тест");
    lemmatizer.addRule("тест", "");
    assert(lemmatizer.getDictionarySize() == 3);

    std::cout << "test_add_rule_and_lemmatize_word passed!\n";
}



void test_lemmatize_single_token() {
    Lemmatizer lemmatizer;
    lemmatizer.addRule("книги", "книга");

    Token wordToken("Книги", "книги", TokenType::WORD, 1, 1);
    Token numToken("2026", "2026", TokenType::NUMBER, 2, 1);
    Token punctToken(".", ".", TokenType::PUNCTUATION, 3, 1);
    Token specialToken("@", "@", TokenType::SPECIAL, 4, 1);

    lemmatizer.lemmatizeToken(wordToken);
    lemmatizer.lemmatizeToken(numToken);
    lemmatizer.lemmatizeToken(punctToken);
    lemmatizer.lemmatizeToken(specialToken);

    // Only WORD tokens should receive a lemma
    assert(wordToken.getLemma() == "книга");
    assert(numToken.getLemma() == "");
    assert(punctToken.getLemma() == "");
    assert(specialToken.getLemma() == "");

    std::cout << "test_lemmatize_single_token passed!\n";
}



void test_lemmatize_tokens_vector() {
    Lemmatizer lemmatizer;
    lemmatizer.addRule("будинки", "будинок");
    lemmatizer.addRule("стояли", "стояти");

    std::string text = "Будинки стояли 100 років.";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    lemmatizer.lemmatizeTokens(tokens);

    // Expected tokens: "Будинки", "стояли", "100", "років", "."
    assert(tokens.size() == 5);
    assert(tokens[0].getLemma() == "будинок");
    assert(tokens[1].getLemma() == "стояти");
    assert(tokens[2].getLemma() == "");       // NUMBER
    assert(tokens[3].getLemma() == "років");  // Unknown WORD stays normalized
    assert(tokens[4].getLemma() == "");       // PUNCTUATION

    std::cout << "test_lemmatize_tokens_vector passed!\n";
}



void test_load_dictionary_from_file() {
    // Non-existent file should return false
    Lemmatizer emptyLem;
    assert(!emptyLem.loadDictionary("non_existent_dictionary_file.txt"));
    assert(emptyLem.getDictionarySize() == 0);

    // Create a temporary dictionary file for testing
    const std::string tempFile = "temp_test_lemma_dict.txt";
    std::ofstream out(tempFile);
    out << "# Comment line that should be ignored\n";
    out << "\n";
    out << "будинків будинок\n";
    out << "йшов йти\n";
    out << "invalid_single_word_line\n";
    out.close();

    Lemmatizer fileLem(tempFile);
    assert(fileLem.getDictionarySize() == 2);
    assert(fileLem.lemmatizeWord("будинків") == "будинок");
    assert(fileLem.lemmatizeWord("йшов") == "йти");

    // Clean up temporary file
    std::remove(tempFile.c_str());

    std::cout << "test_load_dictionary_from_file passed!\n";
}


int main() {
    // Set global locale to Ukrainian UTF-8 for case conversion tests
    std::setlocale(LC_ALL, "uk_UA.UTF-8");

    std::cout << "Running Lemmatizer tests ...\n";

    test_empty_lemmatizer();
    test_add_rule_and_lemmatize_word();
    test_lemmatize_single_token();
    test_lemmatize_tokens_vector();
    test_load_dictionary_from_file();

    std::cout << "All Lemmatizer tests passed successfully!\n";
}