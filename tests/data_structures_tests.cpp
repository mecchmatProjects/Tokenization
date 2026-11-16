#include <iostream>
#include <cassert>
#include <string>

#include "token.h"
#include "sentence.h"
#include "style_profile.h"
#include "text_document.h"



void test_token() {
    // Create a token
    Token t("Apple", TokenType::WORD, 5);

    // Check if getters return the correct values
    assert(t.getText() == "Apple");
    assert(t.getType() == TokenType::WORD);
    assert(t.getPosition() == 5);
    assert(t.getLemma() == ""); // Lemma should be empty initially

    // Check setter
    t.setLemma("apple");
    assert(t.getLemma() == "apple");

    std::cout << "Token tests passed!\n";
}


void test_sentence() {
    Sentence s;
    assert(s.getTokenCount() == 0); // Should be empty

    Token t1("Hello", TokenType::WORD, 1);
    Token t2(",", TokenType::PUNCTUATION, 2);
    
    s.addToken(t1);
    s.addToken(t2);

    assert(s.getTokenCount() == 2);
    assert(s.getTokens()[0].getText() == "Hello");
    assert(s.getTokens()[1].getType() == TokenType::PUNCTUATION);
    
    std::cout << "Sentence tests passed!\n";
}


void test_style_profile() {
    StyleProfile sp;

    // Check defult values
    assert(sp.getTotalWords() == 0);
    assert(sp.getLexicalDiversity() == 0.0);

    // Check setters
    sp.setTotalWords(1500);
    sp.setLexicalDiversity(0.45);

    assert(sp.getTotalWords() == 1500);
    assert(sp.getLexicalDiversity() == 0.45);
    
    std::cout << "StyleProfile tests passed!\n";
}


void test_text_document() {
    TextDocument doc("Kobzar", "Taras Shevchenko");

    assert(doc.getTitle() == "Kobzar");
    assert(doc.getAuthor() == "Taras Shevchenko");
    assert(doc.getSentences().size() == 0);

    Sentence s;
    s.addToken(Token("Word", TokenType::WORD, 1));
    doc.addSentence(s);

    assert(doc.getSentences().size() == 1);

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