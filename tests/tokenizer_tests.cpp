#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "tokenizer.h"



void test_empty_and_whitespace() {
    std::vector<Token> emptyTokens = Tokenizer::tokenize("");
    assert(emptyTokens.empty());

    std::vector<Token> spaceTokens = Tokenizer::tokenize("   \t\n  ");
    assert(spaceTokens.empty());

    std::cout << "test_empty_and_whitespace passed!\n";
}


void test_basic_words_and_punctuation() {
    std::string text = "Hello, World!";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // Expected: "Hello", ",", "World", "!"
    assert(tokens.size() == 4);

    assert(tokens[0].getText() == "Hello");
    assert(tokens[0].getNormalizedText() == "hello");
    assert(tokens[0].getType() == TokenType::WORD);
    assert(tokens[0].getPosition() == 1);
    assert(tokens[0].getSentenceNumber() == 1);

    assert(tokens[1].getText() == ",");
    assert(tokens[1].getType() == TokenType::PUNCTUATION);
    assert(tokens[1].getPosition() == 2);

    assert(tokens[2].getText() == "World");
    assert(tokens[2].getNormalizedText() == "world");
    assert(tokens[2].getType() == TokenType::WORD);
    assert(tokens[2].getPosition() == 3);

    assert(tokens[3].getText() == "!");
    assert(tokens[3].getType() == TokenType::PUNCTUATION);
    assert(tokens[3].getPosition() == 4);
    assert(tokens[3].getSentenceNumber() == 1);

    std::cout << "test_basic_words_and_punctuation passed!\n";
}


void test_multiple_sentences() {
    std::string text = "First sentence. Second one! Third?";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // "First"(1) "sentence"(1) "."(1) "Second"(2) "one"(2) "!"(2) "Third"(3) "?"(3)
    assert(tokens.size() == 8);
    assert(tokens[0].getSentenceNumber() == 1);
    assert(tokens[2].getSentenceNumber() == 1);
    assert(tokens[3].getSentenceNumber() == 2);
    assert(tokens[5].getSentenceNumber() == 2);
    assert(tokens[6].getSentenceNumber() == 3);
    assert(tokens[7].getSentenceNumber() == 3);

    std::cout << "test_multiple_sentences passed!\n";
}


void test_ellipsis() {
    std::string text = "Wait... Really?";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // Expected: "Wait", "...", "Really", "?"
    assert(tokens.size() == 4);
    assert(tokens[1].getText() == "...");
    assert(tokens[1].getType() == TokenType::PUNCTUATION);
    assert(tokens[1].getSentenceNumber() == 1);

    assert(tokens[2].getText() == "Really");
    assert(tokens[2].getSentenceNumber() == 2);

    std::cout << "test_ellipsis passed!\n";
}


void test_hyphen_and_dash() {
    std::string text = "будь-який текст - приклад";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // Expected: "будь-який", "текст", "-", "приклад"
    assert(tokens.size() == 4);
    assert(tokens[0].getText() == "будь-який");
    assert(tokens[0].getType() == TokenType::WORD);

    assert(tokens[2].getText() == "-");
    assert(tokens[2].getType() == TokenType::PUNCTUATION);

    std::cout << "test_hyphen_and_dash passed!\n";
}


void test_numbers() {
    std::string text = "In 2026 pi is 3.14 or 12,5.";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // Expected: "In", "2026", "pi", "is", "3.14", "or", "12,5", "."
    assert(tokens.size() == 8);

    assert(tokens[1].getText() == "2026");
    assert(tokens[1].getType() == TokenType::NUMBER);

    assert(tokens[4].getText() == "3.14");
    assert(tokens[4].getType() == TokenType::NUMBER);
    assert(tokens[4].getSentenceNumber() == 1);

    assert(tokens[6].getText() == "12,5");
    assert(tokens[6].getType() == TokenType::NUMBER);
    assert(tokens[6].getSentenceNumber() == 1);

    std::cout << "test_numbers passed!\n";
}


void test_special_tokens() {
    std::string text = "Price: $100 @ #sale %";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // "@" and "%" should be classified as SPECIAL
    bool foundAt = false;
    bool foundPercent = false;

    for (const Token& t : tokens) {
        if (t.getText() == "@") {
            assert(t.getType() == TokenType::SPECIAL);
            foundAt = true;
        }
        if (t.getText() == "%") {
            assert(t.getType() == TokenType::SPECIAL);
            foundPercent = true;
        }
    }

    assert(foundAt && foundPercent);
    std::cout << "test_special_tokens passed!\n";
}


void test_quotes_and_parentheses() {
    std::string text = "\"Hello!\" (Really?) Next.";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // Sentence 1: ", Hello, !, "
    assert(tokens[0].getText() == "\"");
    assert(tokens[0].getSentenceNumber() == 1);
    assert(tokens[2].getText() == "!");
    assert(tokens[2].getSentenceNumber() == 1);
    assert(tokens[3].getText() == "\"");
    assert(tokens[3].getSentenceNumber() == 1); // Closing quote stays in sentence 1

    // Sentence 2: (, Really, ?, )
    assert(tokens[4].getText() == "(");
    assert(tokens[4].getSentenceNumber() == 2);
    assert(tokens[7].getText() == ")");
    assert(tokens[7].getSentenceNumber() == 2); // Closing parenthesis stays in sentence 2

    // Sentence 3: Next, .
    assert(tokens[8].getText() == "Next");
    assert(tokens[8].getSentenceNumber() == 3);

    std::cout << "test_quotes_and_parentheses passed!\n";
}

void test_dialogue_quotes() {
    std::string text = "\"Hello!\" \"World!\"";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // Sentence 1: ", Hello, !, "
    // Sentence 2: ", World, !, "
    assert(tokens.size() == 8);
    assert(tokens[3].getText() == "\"");
    assert(tokens[3].getSentenceNumber() == 1); // Closing quote stays in sentence 1

    assert(tokens[4].getText() == "\"");
    assert(tokens[4].getSentenceNumber() == 2); // Opening quote starts sentence 2
    assert(tokens[5].getText() == "World");
    assert(tokens[5].getSentenceNumber() == 2);

    std::cout << "test_dialogue_quotes passed!\n";
}


void test_ukrainian_text() {
    std::string text = "Це п'ять м'ячів. Сонце світило!";
    std::vector<Token> tokens = Tokenizer::tokenize(text);

    // "Це", "п'ять", "м'ячів", ".", "Сонце", "світило", "!"
    assert(tokens.size() == 7);

    assert(tokens[0].getText() == "Це");
    assert(tokens[0].getNormalizedText() == "це");
    assert(tokens[0].getType() == TokenType::WORD);

    assert(tokens[1].getText() == "п'ять");
    assert(tokens[1].getNormalizedText() == "п'ять");
    assert(tokens[1].getType() == TokenType::WORD);

    assert(tokens[2].getText() == "м'ячів");
    assert(tokens[2].getType() == TokenType::WORD);

    assert(tokens[4].getText() == "Сонце");
    assert(tokens[4].getNormalizedText() == "сонце");
    assert(tokens[4].getSentenceNumber() == 2);

    std::cout << "test_ukrainian_text passed!\n";
}


int main() {
    // Set global locale to Ukrainian UTF-8 for case conversion tests
    std::setlocale(LC_ALL, "uk_UA.UTF-8");

    std::cout << "Running Tokenizer tests ...\n";

    test_empty_and_whitespace();
    test_basic_words_and_punctuation();
    test_multiple_sentences();
    test_ellipsis();
    test_hyphen_and_dash();
    test_numbers();
    test_special_tokens();
    test_quotes_and_parentheses();
    test_dialogue_quotes();
    test_ukrainian_text();

    std::cout << "All Tokenizer tests passed successfully!\n";
    return 0;
}