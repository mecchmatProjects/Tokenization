#include "tokenization/RuleBasedTokenizer.h"
#include "tokenization/TokenizerFactory.h"
#include "tokenization/RuleDetection.h"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {
using Expected = std::pair<std::string, TokenType>;
using Type = TokenType;

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void check(const std::string& text, const std::vector<Expected>& expected) {
    const auto strategy = TokenizerFactory::create(TokenizerType::RuleBased);
    const auto tokens = strategy->tokenize(text);
    require(tokens.size() == expected.size(), "Token count for: " + text);
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        require(tokens[i].text == expected[i].first && tokens[i].type == expected[i].second,
                "Token " + std::to_string(i) + " for: " + text);
        require(tokens[i].position == i && tokens[i].tokenId == -1,
                "Invalid token metadata");
    }
}
}

int main() {
    try {
        check("", {});
        check(u8" \t\r\n\u00A0\u2003", {});
        check("Hello, world!", {{"Hello", Type::Word}, {",", Type::Punctuation},
                               {"world", Type::Word}, {"!", Type::Punctuation}});
        check("don't mother-in-law COVID-19 abc123 C++ c++ #machine_learning",
              {{"don't", Type::Word}, {"mother-in-law", Type::Word}, {"COVID-19", Type::Word},
               {"abc123", Type::Word}, {"C++", Type::Special}, {"c++", Type::Special},
               {"#machine_learning", Type::Special}});
        check("C# .NET c++/cli C++/CLI.",
              {{"C#", Type::Special}, {".NET", Type::Special}, {"c++/cli", Type::Special},
               {"C++/CLI", Type::Special}, {".", Type::Punctuation}});
        check(".NETCore C++17 XC++ C#tag",
              {{".", Type::Punctuation}, {"NETCore", Type::Word}, {"C", Type::Word},
               {"+", Type::Special}, {"+17", Type::Number},
               {"XC", Type::Word}, {"+", Type::Special}, {"+", Type::Special},
               {"C", Type::Word}, {"#tag", Type::Special}});
        check("(.net), C++!", {{"(", Type::Punctuation}, {".net", Type::Special},
                               {")", Type::Punctuation}, {",", Type::Punctuation},
                               {"C++", Type::Special}, {"!", Type::Punctuation}});
        check(u8"п'ять п’ять пʼять Привіт світе! 你好世界！",
              {{u8"п'ять", Type::Word}, {u8"п’ять", Type::Word}, {u8"пʼять", Type::Word},
               {u8"Привіт", Type::Word}, {u8"світе", Type::Word}, {"!", Type::Punctuation},
               {u8"你好世界", Type::Word}, {u8"！", Type::Punctuation}});
        check("10 -15 +2.5 .75 -.5 1e-5 3.14E+10 10% $100",
              {{"10", Type::Number}, {"-15", Type::Number}, {"+2.5", Type::Number},
               {".75", Type::Number}, {"-.5", Type::Number}, {"1e-5", Type::Number},
               {"3.14E+10", Type::Number}, {"10", Type::Number}, {"%", Type::Special},
               {"$", Type::Special}, {"100", Type::Number}});
        check("10-5 1e+ x=-2.5",
              {{"10", Type::Number}, {"-", Type::Punctuation}, {"5", Type::Number},
               {"1", Type::Number}, {"e", Type::Word}, {"+", Type::Special},
               {"x", Type::Word}, {"=", Type::Special}, {"-2.5", Type::Number}});
        check("Price: 12.50.", {{"Price", Type::Word}, {":", Type::Punctuation},
                                {"12.50", Type::Number}, {".", Type::Punctuation}});
        check("Email: test@example.com, first.last+tag@sub.example.org!",
              {{"Email", Type::Word}, {":", Type::Punctuation}, {"test@example.com", Type::Special},
               {",", Type::Punctuation}, {"first.last+tag@sub.example.org", Type::Special},
               {"!", Type::Punctuation}});
        check("user@localhost a..b@example.com @bad.com",
              {{"user", Type::Word}, {"@", Type::Special}, {"localhost", Type::Word},
               {"a", Type::Word}, {".", Type::Punctuation}, {".", Type::Punctuation},
               {"b", Type::Word}, {"@", Type::Special}, {"example", Type::Word},
               {".", Type::Punctuation}, {"com", Type::Word}, {"@", Type::Special},
               {"bad", Type::Word}, {".", Type::Punctuation}, {"com", Type::Word}});
        check("Visit https://example.com. HTTP://example.org/a?q=1&b=2! www.example.net/path,",
              {{"Visit", Type::Word}, {"https://example.com", Type::Special}, {".", Type::Punctuation},
               {"HTTP://example.org/a?q=1&b=2", Type::Special}, {"!", Type::Punctuation},
               {"www.example.net/path", Type::Special}, {",", Type::Punctuation}});
        check("(https://example.com/a_(b)). [https://example.org]",
              {{"(", Type::Punctuation}, {"https://example.com/a_(b)", Type::Special},
               {")", Type::Punctuation}, {".", Type::Punctuation}, {"[", Type::Punctuation},
               {"https://example.org", Type::Special}, {"]", Type::Punctuation}});
        check("\"https://example.com\" https:// www.",
              {{"\"", Type::Punctuation}, {"https://example.com", Type::Special},
               {"\"", Type::Punctuation}, {"https", Type::Word}, {":", Type::Punctuation},
               {"/", Type::Punctuation}, {"/", Type::Punctuation}, {"www", Type::Word},
               {".", Type::Punctuation}});
        check("Hello...?! --", {{"Hello", Type::Word}, {".", Type::Punctuation}, {".", Type::Punctuation},
                                 {".", Type::Punctuation}, {"?", Type::Punctuation}, {"!", Type::Punctuation},
                                 {"-", Type::Punctuation}, {"-", Type::Punctuation}});
        check(std::string("a\0b", 3), {{"a", Type::Word}, {std::string(1, '\0'), Type::Unknown}, {"b", Type::Word}});

        const auto sentences = RuleBasedTokenizer().tokenize("Price 12.50! Visit https://example.com. Next?");
        require(sentences.size() == 8 && sentences[1].sentenceNumber == 1 &&
                    sentences[3].sentenceNumber == 2 && sentences[4].sentenceNumber == 2 &&
                    sentences[6].sentenceNumber == 3, "Decimal/URL sentence boundaries");
        const auto unicodeSentences = RuleBasedTokenizer().tokenize(u8"你好！Привіт。Bye?");
        require(unicodeSentences[2].sentenceNumber == 2 && unicodeSentences[4].sentenceNumber == 3,
                "Unicode sentence boundaries");
        const auto punctuation = RuleBasedTokenizer().tokenize("Hello...?! Next");
        require(punctuation.back().sentenceNumber == 2, "Repeated punctuation must end a sentence once");
        const auto invalid = std::string("\xC3", 1);
        bool rejected = false;
        try { RuleBasedTokenizer().tokenize(invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Invalid UTF-8 must be rejected");

        const std::string mentionText = "@user";
        const auto mentionCharacters = tokenization_detail::decodeUtf8(mentionText);
        const RuleContext mentionContext{mentionText, mentionCharacters, {}};
        auto extendedRules = RuleDetection::rules();
        extendedRules.insert(extendedRules.begin(), [](const RuleContext& context, std::size_t start) -> RuleMatch {
            if (context.characters[start].value != U'@') return {};
            auto end = start + 1;
            while (end < context.characters.size() &&
                   tokenization_detail::classify(context.characters[end].value) == TokenType::Word) ++end;
            return end > start + 1 ? RuleMatch{end - start, TokenType::Special} : RuleMatch{};
        });
        const auto mention = RuleDetection::matchFirst(mentionContext, 0, extendedRules);
        require(mention.length == 5 && mention.type == Type::Special,
                "Adding a lambda must extend recognition without changing the tokenizer");
        require(RuleDetection::matchFirst(mentionContext, 0, RuleDetection::rules()).length == 0,
                "Default rules must not recognize a mention as a compound token");

        bool laterRuleCalled = false;
        const std::vector<DetectionRule> priorityRules = {
            [](const RuleContext&, std::size_t) -> RuleMatch { return {}; },
            [](const RuleContext&, std::size_t) -> RuleMatch { return {1, Type::Special}; },
            [&laterRuleCalled](const RuleContext&, std::size_t) -> RuleMatch {
                laterRuleCalled = true;
                return {2, Type::Word};
            },
        };
        require(RuleDetection::matchFirst(mentionContext, 0, priorityRules).length == 1 && !laterRuleCalled,
                "Nonmatching rules must be skipped; the first match must stop evaluation");
        require(RuleDetection::matchFirst(mentionContext, 0, {}).length == 0,
                "An empty rule list must report no match");
        bool badRuleRejected = false;
        try {
            RuleDetection::matchFirst(mentionContext, 0, {
                [](const RuleContext&, std::size_t) -> RuleMatch { return {100, Type::Word}; },
            });
        } catch (const std::logic_error&) { badRuleRejected = true; }
        require(badRuleRejected, "Invalid rule results must not allow reading outside the input");

        std::string large;
        for (int i = 0; i < 10000; ++i) large += "Hello test@example.com -12.50! ";
        require(RuleBasedTokenizer().tokenize(large).size() == 40000, "Large input");
        const std::string longWord(100000, 'a');
        check(longWord, {{longWord, Type::Word}});
        std::cout << "Rule-based tokenizer checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
