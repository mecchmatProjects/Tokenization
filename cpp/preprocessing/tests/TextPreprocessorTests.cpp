#include "preprocessing/TextPreprocessor.h"

#include <iostream>
#include <stdexcept>

namespace {
int failures = 0;

void expect(const char* name, const std::string& input, const std::string& expected,
            PreprocessingConfig config = {}) {
    const TextPreprocessor implementation(config);
    const Preprocessor& processor = implementation;
    if (processor.process(input) != expected) {
        std::cerr << "FAIL: " << name << '\n';
        ++failures;
    }
}

void expectInvalid(const std::string& input) {
    try {
        TextPreprocessor().process(input);
        std::cerr << "FAIL: invalid UTF-8 accepted\n";
        ++failures;
    } catch (const std::invalid_argument&) {
    }
}
}

int main() {
    expect("empty", "", "");
    expect("whitespace only", " \t\r\n\v\f", "");
    expect("horizontal whitespace", "  Hello\t \tworld  ", "Hello world");
    expect("normalize CRLF and CR", "Hello\r\nworld\rnext\nline", "Hello\nworld\nnext\nline");
    expect("blank lines", "a\r\n\r\nb", "a\n\nb");
    expect("Unicode spaces", u8"a\u00A0\u2003\u202Fb", "a b");
    expect("Unicode newlines", u8"a\u0085b\u2028c\u2029d", "a\nb\nc\nd");
    expect("vertical controls", "a\vb\fc", "a\nb\nc");
    expect("preserve case by default", u8"ПРИВІТ World", u8"ПРИВІТ World");
    expect("punctuation preserved", "don't https://example.com user@example.com -12.50!",
           "don't https://example.com user@example.com -12.50!");
    expect("embedded NUL", std::string("a\0b", 3), std::string("a\0b", 3));

    PreprocessingConfig config;
    config.lowercase = true;
    expect("Unicode lowercase", u8"ПРИВІТ МИР ÉCOLE World 你好", u8"привіт мир école world 你好", config);
    expect("combining marks", u8"E\u0301", u8"e\u0301", config);

    config = {};
    config.newlineMode = NewlineMode::ReplaceWithSpace;
    expect("replace and collapse", "Hello \r\n \tworld", "Hello world", config);
    config.collapseHorizontalWhitespace = false;
    expect("CRLF replaced once", "a\r\nb", "a b", config);
    expect("no collapse", "a \r\n b", "a   b", config);

    config = {};
    config.trimWhitespace = false;
    expect("no trim", " \ta  \r\n ", " a \n ", config);
    expect("space only without trim", " \t ", " ", config);

    config = {};
    config.collapseHorizontalWhitespace = false;
    expect("trim independently", " \ta \t b\r\n", "a \t b", config);

    config = {};
    config.newlineMode = NewlineMode::Preserve;
    expect("preserve internal CRLF", "a\r\nb\rc", "a\r\nb\rc", config);
    expect("preserve with trim", "\r\na\r\n", "a", config);
    config.trimWhitespace = false;
    config.collapseHorizontalWhitespace = false;
    const std::string unchanged = u8" \tПРИВІТ\r\n\u00A0你好\u2028 ";
    expect("all transformations disabled", unchanged, unchanged, config);

    expectInvalid(std::string("\xFF", 1));
    expectInvalid(std::string("\xC3", 1));
    expectInvalid(std::string("\xC0\xAF", 2));
    expectInvalid(std::string("\xED\xA0\x80", 3));
    expectInvalid(std::string("\xF4\x90\x80\x80", 4));

    std::string largeInput;
    std::string largeExpected;
    for (int i = 0; i < 10000; ++i) {
        largeInput += "Hello\t\tworld\r\n";
        largeExpected += "Hello world\n";
    }
    largeExpected.pop_back();
    expect("large input", largeInput, largeExpected);

    if (failures != 0) {
        return 1;
    }
    std::cout << "All preprocessing checks passed\n";
    return 0;
}
