#include <iostream>
#include <cassert>
#include <string>

#include "preprocessor.h"



void test_normalize_spaces() {
    std::string raw = "Hello     world!  \t This is   a test.   ";
    // Default flags: do not remove punctuation, do not lower case
    std::string cleaned = Preprocessor::process(raw);
    
    assert(cleaned == "Hello world! This is a test.");
    std::cout << "test_normalize_spaces passed!\n";
}


void test_normalize_quotes_and_dashes() {
    std::string raw = "Він сказав: «Привіт» — і пішов.";
    std::string cleaned = Preprocessor::process(raw);
    
    // Check if typographical symbols were converted to standard ASCII
    assert(cleaned == "Він сказав: \"Привіт\" - і пішов.");
    std::cout << "test_normalize_quotes_and_dashes passed!\n";
}


void test_remove_punctuation() {
    std::string raw = "Hello, world! How are you?";
    // Flags: removePunctuation = true, toLowerCase = false
    std::string cleaned = Preprocessor::process(raw, true, false);
    
    assert(cleaned == "Hello world How are you");
    std::cout << "test_remove_punctuation passed!\n";
}


void test_to_lower_case() {
    std::string raw = "LOUD NOISES";
    // Flags: removePunctuation = false, toLowerCase = true
    std::string cleaned = Preprocessor::process(raw, false, true);
    
    assert(cleaned == "loud noises");
    std::cout << "test_to_lower_case passed!\n";
}


void test_line_breaks() {
    std::string raw = "Line one\r\nLine two\rLine three\nLine four";
    std::string cleaned = Preprocessor::process(raw);
    
    assert(cleaned == "Line one\nLine two\nLine three\nLine four");
    std::cout << "test_line_breaks passed!\n";
}



int main() {
    std::cout << "Running Preprocessor tests ...\n";

    test_normalize_spaces();
    test_normalize_quotes_and_dashes();
    test_remove_punctuation();
    test_to_lower_case();
    test_line_breaks();

    std::cout << "All Preprocessor tests passed successfully!\n";
    return 0;
}