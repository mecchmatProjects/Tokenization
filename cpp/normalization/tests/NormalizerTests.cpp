#include "normalization/Normalizer.h"
#include "tokenization/RuleBasedTokenizer.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

std::vector<Token> run(const std::string& text, const NormalizationConfig& config,
                       const StopWordDictionary& words = {}) {
    return Normalizer().normalize(RuleBasedTokenizer().tokenize(text), config, words);
}

bool equal(const std::vector<Token>& a, const std::vector<Token>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].text != b[i].text || a[i].type != b[i].type || a[i].position != b[i].position ||
            a[i].sentenceNumber != b[i].sentenceNumber || a[i].tokenId != b[i].tokenId) return false;
    }
    return true;
}

struct TestFile {
    std::string path = "normalizer-stopwords-" + std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count()) + ".txt";
    ~TestFile() { std::remove(path.c_str()); }
    void write(const std::string& text) {
        std::ofstream out(path, std::ios::binary);
        out.write(text.data(), static_cast<std::streamsize>(text.size()));
        if (!out) throw std::runtime_error("Cannot create test dictionary");
    }
};
}

int main() {
    try {
        NormalizationConfig config;
        StopWordDictionary words;
        auto input = RuleBasedTokenizer().tokenize("The price is 12.50! Next.");
        input[0].tokenId = 123;
        const auto original = input;
        require(equal(Normalizer().normalize(input, config, words), input), "Disabled options must preserve all fields");
        require(Normalizer().normalize({}, config, words).empty(), "Empty token list");

        config.unicode = UnicodeNormalization::NFC;
        const std::vector<Token> decomposed = {Token(u8"e\u0301", TokenType::Word, 8, 3, 99)};
        const auto composed = Normalizer().normalize(decomposed, config, words);
        require(composed[0].text == u8"é" && composed[0].tokenId == -1 && composed[0].position == 8 &&
                    composed[0].sentenceNumber == 3 && decomposed[0].text == u8"e\u0301", "NFC and metadata");
        config.unicode = UnicodeNormalization::NFKC;
        require(run(u8"ﬁ ＡＢＣ", config)[0].text == "fi" && run(u8"ﬁ ＡＢＣ", config)[1].text == "ABC", "NFKC compatibility forms");
        const auto nul = Normalizer().normalize({Token(std::string("e\0x", 3), TokenType::Unknown)}, config, words);
        require(nul[0].text == std::string("e\0x", 3), "Unicode mapping must preserve embedded NUL");
        bool invalid = false;
        try { Normalizer().normalize({Token(std::string("\xFF", 1), TokenType::Word)}, config, words); }
        catch (const std::invalid_argument&) { invalid = true; }
        require(invalid, "Invalid UTF-8 must be rejected during Unicode mapping");

        config = {};
        config.replaceNumbers = true;
        auto numbers = run("10 -12.5 1e-5!", config);
        require(numbers.size() == 4 && numbers[0].text == "<NUM>" && numbers[1].text == "<NUM>" &&
                    numbers[2].type == TokenType::Special && numbers[3].type == TokenType::Punctuation, "Number markers");

        config = {};
        config.replaceUrls = true;
        auto urls = run("https://example.com/a@b.com user@example.com @", config);
        require(urls[0].text == "<URL>" && urls[1].text == "user@example.com" && urls[2].text == "@", "Independent URL option");
        config.replaceUrls = false;
        config.replaceEmails = true;
        auto emails = run("https://example.com/a@b.com user@example.com @ #tag", config);
        require(emails[0].text == "https://example.com/a@b.com" && emails[1].text == "<EMAIL>" &&
                    emails[2].text == "@" && emails[3].text == "#tag", "Email detection must not consume URLs or arbitrary @ tokens");
        config.replaceUrls = true;
        const auto markers = run("HTTP://example.com. user@example.com!", config);
        require(markers.size() == 4 && markers[0].text == "<URL>" && markers[2].text == "<EMAIL>", "Markers and punctuation");
        require(equal(Normalizer().normalize(markers, config, words), markers), "Marker replacement must be idempotent");

        words.add("the"); words.add("is"); words.add(u8"і");
        config = {};
        config.removeStopWords = true;
        auto filtered = Normalizer().normalize(input, config, words);
        require(filtered.size() == 5 && filtered[0].text == "price" && filtered[0].position == 1 &&
                    filtered[3].text == "Next" && filtered[3].sentenceNumber == 2, "Stop words and original positions");
        require(equal(input, original), "Input tokens must not be modified");
        config.stopWordsCaseSensitive = true;
        require(run("The the", config, words).size() == 1, "Case-sensitive stop words");
        config.stopWordsCaseSensitive = false;
        words.add("STRASSE");
        require(run(u8"Straße", config, words).empty(), "Unicode case folding for comparison");
        words.add(u8"é");
        config.unicode = UnicodeNormalization::NFC;
        require(run(u8"e\u0301", config, words).empty(), "Dictionary and tokens must use the same Unicode form");
        words.add("<URL>");
        config.replaceUrls = true;
        require(run("https://example.com", config, words)[0].text == "<URL>", "Stop words must not remove markers");

        config = {};
        config.removePunctuation = true;
        auto noPunctuation = run("Hello! Next?", config);
        require(noPunctuation.size() == 2 && noPunctuation[1].position == 2 && noPunctuation[1].sentenceNumber == 2,
                "Punctuation removal must retain sentence and position metadata");
        config.removeStopWords = true;
        config.replaceNumbers = true;
        config.replaceUrls = true;
        config.replaceEmails = true;
        config.unicode = UnicodeNormalization::NFC;
        auto combined = run("The price is 12.50! https://example.com user@example.com", config, words);
        require(combined.size() == 4 && combined[0].text == "price" && combined[1].text == "<NUM>" &&
                    combined[2].text == "<URL>" && combined[3].text == "<EMAIL>", "Combined options");

        TestFile file;
        file.write(std::string("\xEF\xBB\xBF") + u8" the \r\nis\r\n\r\n\u00A0і\u00A0\nTHE\n");
        StopWordDictionary loaded;
        loaded.loadFromFile(file.path);
        require(loaded.words().size() == 4 && loaded.contains("the") && loaded.contains(u8"і"), "UTF-8 BOM, CRLF and Unicode trimming");
        file.write("two words\n");
        bool badLine = false;
        try { loaded.loadFromFile(file.path); } catch (const std::invalid_argument&) { badLine = true; }
        require(badLine && loaded.contains("the"), "Failed loads must retain the existing dictionary");
        file.write(std::string("\xFF", 1));
        invalid = false;
        try { loaded.loadFromFile(file.path); } catch (const std::invalid_argument&) { invalid = true; }
        require(invalid, "Invalid dictionary UTF-8");
        bool missing = false;
        try { loaded.loadFromFile(file.path + ".missing"); } catch (const std::runtime_error&) { missing = true; }
        require(missing, "Missing dictionary file");
        file.write("");
        loaded.loadFromFile(file.path);
        require(loaded.words().empty(), "Successful loading replaces the dictionary");
        std::cout << "Normalizer checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
