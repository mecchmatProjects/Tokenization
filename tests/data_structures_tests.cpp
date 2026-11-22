#include <iostream>
#include <cassert>
#include <string>

#include "token.h"
#include "sentence.h"
#include "style_profile.h"
#include "text_document.h"



void test_token() {
    // Create a token
    Token t("Apple", "apple", TokenType::WORD, 5, 1);

    // Check if getters return the correct values
    assert(t.getText() == "Apple");
    assert(t.getNormalizedText() == "apple");
    assert(t.getType() == TokenType::WORD);
    assert(t.getSentenceNumber() == 1);

    std::cout << "Token tests passed!\n";
}


void test_sentence() {
    Sentence s(1, "Hello, world.");
    assert(s.getNumber() == 1);
    
    s.addToken(Token("Hello", "hello", TokenType::WORD, 1, 1));
    s.addToken(Token(",", ",", TokenType::PUNCTUATION, 2, 1));

    assert(s.getTokenCount() == 2);
    assert(s.getWordCount() == 1);
    assert(s.getPunctuationCount() == 1);
    
    std::cout << "Sentence tests passed!\n";
}


void test_style_profile() {
    StyleProfile sp;

    // setLexicalStats(words, unique, lemmas, diversity, avgWordLen)
    sp.setLexicalStats(100, 50, 45, 0.5, 4.2);
    
    // Assertions must exactly match the values we just set above
    assert(sp.getTotalWords() == 100);
    assert(sp.getLexicalDiversity() == 0.5);
    
    std::cout << "StyleProfile tests passed!\n";
}


void test_text_document() {
    TextDocument doc("Kobzar", "Taras Shevchenko", 1840, "Ukrainian", "Текст Кобзаря...");

    assert(doc.getTitle() == "Kobzar");
    assert(doc.getAuthor() == "Taras Shevchenko");
    assert(doc.getSentences().size() == 0);
    assert(doc.getYear() == 1840);
    assert(doc.getLanguage() == "Ukrainian");

    std::cout << "TextDocument tests passed!\n";
}

int main() {
   std::cout << "Running Data Structures tests...\n";
    
    test_token();
    test_sentence();
    test_style_profile();
    test_text_document();
    
    std::cout << "All tests passed successfully!\n";
    return 0; 
}