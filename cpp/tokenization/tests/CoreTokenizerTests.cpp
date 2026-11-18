#include "tokenization/TokenizerFactory.h"
#include "tokenization/WhitespaceTokenizer.h"
#include "tokenization/WordPunctuationTokenizer.h"
#include <iostream>
#include <stdexcept>
#include <utility>

namespace {
using ExpectedToken = std::pair<std::string, TokenType>;
const TokenizerType strategies[] = {
    TokenizerType::Whitespace, TokenizerType::WordPunctuation, TokenizerType::RuleBased
};

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void expect(const std::vector<Token>& actual, const std::vector<ExpectedToken>& expected) {
    require(actual.size() == expected.size(), "Unexpected token count");
    for (std::size_t i = 0; i < expected.size(); ++i) {
        require(actual[i].text == expected[i].first && actual[i].type == expected[i].second,
                "Unexpected text or type at token " + std::to_string(i));
    }
}

void whitespace() {
    expect(WhitespaceTokenizer().tokenize(u8" \tПривіт,\u00A0world!\r\n42 "), {
        {u8"Привіт,", TokenType::Word}, {"world!", TokenType::Word}, {"42", TokenType::Number}
    });
}

void basic() {
    expect(WordPunctuationTokenizer().tokenize(u8"Привіт, світе! 42 + $"), {
        {u8"Привіт", TokenType::Word}, {",", TokenType::Punctuation},
        {u8"світе", TokenType::Word}, {"!", TokenType::Punctuation},
        {"42", TokenType::Number}, {"+", TokenType::Special}, {"$", TokenType::Special}
    });
}

void metadata() {
    for (const auto type : strategies) {
        const auto tokenizer = TokenizerFactory::create(type);
        const auto tokens = tokenizer->tokenize("Hello! Next?");
        require(!tokens.empty() && tokens.front().sentenceNumber == 1 && tokens.back().sentenceNumber == 2,
                "Sentence numbers must advance after a sentence boundary");
        for (std::size_t i = 0; i < tokens.size(); ++i) {
            require(tokens[i].position == i && tokens[i].tokenId == -1,
                    "Tokens must have consecutive positions and unassigned vocabulary IDs");
        }
        const auto repeated = tokenizer->tokenize("Again");
        require(repeated.size() == 1 && repeated[0].position == 0 && repeated[0].sentenceNumber == 1,
                "Metadata must restart for each document");
    }
}

void empty() {
    for (const auto type : strategies) {
        const auto tokenizer = TokenizerFactory::create(type);
        require(tokenizer->tokenize("").empty(), "Empty input must produce no tokens");
        require(tokenizer->tokenize(u8" \t\r\n\u00A0").empty(), "Whitespace-only input must produce no tokens");
    }
}

void factory() {
    const std::string text = "Hello, test@example.com!";
    expect(TokenizerFactory::create(TokenizerType::Whitespace)->tokenize(text), {
        {"Hello,", TokenType::Word}, {"test@example.com!", TokenType::Word}
    });
    expect(TokenizerFactory::create(TokenizerType::WordPunctuation)->tokenize(text), {
        {"Hello", TokenType::Word}, {",", TokenType::Punctuation}, {"test", TokenType::Word},
        {"@", TokenType::Special}, {"example", TokenType::Word}, {".", TokenType::Punctuation},
        {"com", TokenType::Word}, {"!", TokenType::Punctuation}
    });
    expect(TokenizerFactory::create(TokenizerType::RuleBased)->tokenize(text), {
        {"Hello", TokenType::Word}, {",", TokenType::Punctuation},
        {"test@example.com", TokenType::Special}, {"!", TokenType::Punctuation}
    });
    bool rejected = false;
    try { TokenizerFactory::create(static_cast<TokenizerType>(99)); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "Unknown strategy must be rejected");
}

void invalidUtf8() {
    for (const auto type : strategies) {
        bool rejected = false;
        try { TokenizerFactory::create(type)->tokenize(std::string("\xFF", 1)); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Invalid UTF-8 must be rejected");
    }
}
}

int main(int argc, char* argv[]) {
    const std::pair<const char*, void(*)()> cases[] = {
        {"whitespace", whitespace}, {"basic", basic}, {"metadata", metadata},
        {"empty", empty}, {"factory", factory}, {"invalid_utf8", invalidUtf8}
    };
    try {
        if (argc == 2) {
            for (const auto& test : cases) {
                if (argv[1] == std::string(test.first)) {
                    test.second();
                    std::cout << "PASS: " << test.first << '\n';
                    return 0;
                }
            }
        }
        throw std::invalid_argument("Expected a test case name");
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
