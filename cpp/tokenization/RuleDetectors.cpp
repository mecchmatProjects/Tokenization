#include "RuleDetectors.h"
#include "PredefinedWords.h"
#include <utf8proc.h>
#include "TokenizerUtils.h"


namespace {
using tokenization_detail::Character;
using tokenization_detail::classify;
using Characters = std::vector<Character>;

bool digit(char32_t c) { return c >= U'0' && c <= U'9'; }
bool letter(char32_t c) {
    return (c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z');
}
bool asciiAlnum(char32_t c) { return letter(c) || digit(c); }
bool wordPart(char32_t c) {
    return classify(c) == TokenType::Word || classify(c) == TokenType::Number;
}
bool connector(char32_t c) {
    return c == U'-' || c == U'\'' || c == 0x2019 || c == 0x02BC;
}
bool emailLocal(char32_t c) {
    if (asciiAlnum(c)) return true;
    static const std::string symbols = ".!#$%&'*+-/=?^_`{|}~";
    return c < 128 && symbols.find(static_cast<char>(c)) != std::string::npos;
}

std::vector<std::size_t> emailSpans(const Characters& chars) {
    std::vector<std::size_t> ends(chars.size(), 0);
    for (std::size_t at = 0; at < chars.size(); ++at) {
        if (chars[at].value != U'@') continue;
        std::size_t start = at;
        while (start > 0 && emailLocal(chars[start - 1].value)) --start;
        if (start == at || chars[start].value == U'.' || chars[at - 1].value == U'.' ||
            (start > 0 && (chars[start - 1].value == U'@' || wordPart(chars[start - 1].value)))) continue;
        bool valid = true;
        for (std::size_t i = start + 1; i < at; ++i) {
            if (chars[i].value == U'.' && chars[i - 1].value == U'.') valid = false;
        }
        std::size_t end = at + 1;
        std::size_t labels = 0;
        while (end < chars.size() && asciiAlnum(chars[end].value)) {
            ++labels;
            do { ++end; }
            while (end < chars.size() && (asciiAlnum(chars[end].value) || chars[end].value == U'-'));
            if (chars[end - 1].value == U'-') valid = false;
            if (end + 1 < chars.size() && chars[end].value == U'.' && asciiAlnum(chars[end + 1].value)) {
                ++end;
            } else {
                break;
            }
        }
        if (end < chars.size() && (chars[end].value == U'@' || chars[end].value == U'_' ||
                                  classify(chars[end].value) == TokenType::Word)) valid = false;
        if (valid && labels >= 2) ends[start] = end;
    }
    return ends;
}

bool startsWith(const Characters& chars, std::size_t start, const std::string& prefix) {
    if (chars.size() - start < prefix.size()) return false;
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        char32_t c = chars[start + i].value;
        if (c >= U'A' && c <= U'Z') c += U'a' - U'A';
        if (c != static_cast<unsigned char>(prefix[i])) return false;
    }
    return true;
}

std::size_t urlEnd(const Characters& chars, std::size_t start) {
    std::size_t host = start;
    const char* prefixes[] = {"https://", "http://", "www."};
    for (const auto prefix : prefixes) {
        const std::string pattern(prefix);
        if (!pattern.empty() && startsWith(chars, start, pattern)) {
            host += pattern.size();
            break;
        }
    }
    if (host == start) return start;
    if (host == chars.size() || !(wordPart(chars[host].value) || chars[host].value == U'[')) return start;

    std::size_t end = host;
    int parentheses = 0;
    int brackets = 0;
    while (end < chars.size()) {
        const auto c = chars[end].value;
        if (classify(c) == TokenType::Whitespace || c == U'<' || c == U'>' ||
            c == U'"' || c == U'\'' || c == U'{' || c == U'}' ||
            c == 0x2018 || c == 0x2019 || c == 0x201C || c == 0x201D ||
            c == 0x3002 || c == 0xFF01 || c == 0xFF1F || c == 0xFF0C) break;
        if (c == U'(') ++parentheses;
        if (c == U'[') ++brackets;
        if (c == U')' && --parentheses < 0) break;
        if (c == U']' && --brackets < 0) break;
        ++end;
    }
    while (end > host) {
        const auto c = chars[end - 1].value;
        bool trailingPunctuation = false;
        const char32_t punctuationCharacters[] = {U'.', U',', U'!', U'?', U':', U';', 0x2026};
        for (const auto punctuation : punctuationCharacters) {
            if (c == punctuation) trailingPunctuation = true;
        }
        if (!trailingPunctuation) break;
        --end;
    }
    return end > host ? end : start;
}

std::size_t numberEnd(const Characters& chars, std::size_t start) {
    std::size_t end = start;
    if (chars[end].value == U'+' || chars[end].value == U'-') {
        if (start > 0 && (wordPart(chars[start - 1].value) ||
            chars[start - 1].value == U')' || chars[start - 1].value == U']' ||
            chars[start - 1].value == U'-')) return start;
        ++end;
    }
    const std::size_t digitsStart = end;
    while (end < chars.size() && digit(chars[end].value)) ++end;
    bool hasDigits = end > digitsStart;
    if (end + 1 < chars.size() && chars[end].value == U'.' && digit(chars[end + 1].value)) {
        ++end;
        while (end < chars.size() && digit(chars[end].value)) ++end;
        hasDigits = true;
    }
    if (!hasDigits) return start;
    if (end < chars.size() && (chars[end].value == U'e' || chars[end].value == U'E')) {
        std::size_t exponent = end + 1;
        if (exponent < chars.size() && (chars[exponent].value == U'+' || chars[exponent].value == U'-')) ++exponent;
        const auto exponentStart = exponent;
        while (exponent < chars.size() && digit(chars[exponent].value)) ++exponent;
        if (exponent > exponentStart) end = exponent;
    }
    return end;
}
}

namespace RuleDetectors {
RuleMatch url(const RuleContext& context, std::size_t start) {
    const auto end = urlEnd(context.characters, start);
    return {end - start, TokenType::Special};
}

RuleMatch email(const RuleContext& context, std::size_t start) {
    auto found = context.spanCache.find("email");
    if (found == context.spanCache.end()) {
        found = context.spanCache.emplace("email", emailSpans(context.characters)).first;
    }
    const auto end = found->second[start];
    return end > start ? RuleMatch{end - start, TokenType::Special} : RuleMatch{};
}

RuleMatch predefinedWord(const RuleContext& context, std::size_t start) {
    const auto& chars = context.characters;
    if (start >= chars.size()) return {};
    auto isWordBoundary = [](char32_t c) { return !wordPart(c) && c != U'_'; };
    if (start > 0 && !isWordBoundary(chars[start - 1].value)) return {};

    RuleMatch best;
    for (const auto& entry : PredefinedWords::dictionary()) {
        const auto length = entry.text.size();
        if (length == 0 || length <= best.length || length > chars.size() - start) continue;
        const auto end = start + length;
        if (end < chars.size() && !isWordBoundary(chars[end].value)) continue;

        bool matches = true;
        for (std::size_t i = 0; i < length; ++i) {
            if (utf8proc_tolower(static_cast<utf8proc_int32_t>(chars[start + i].value)) !=
                utf8proc_tolower(static_cast<utf8proc_int32_t>(entry.text[i]))) {
                matches = false;
                break;
            }
        }
        if (matches) best = {length, entry.type};
    }
    return best;
}

RuleMatch hashtag(const RuleContext& context, std::size_t start) {
    const auto& chars = context.characters;
    if (chars[start].value != U'#' || start + 1 == chars.size() || !wordPart(chars[start + 1].value)) return {};
    auto end = start + 2;
    while (end < chars.size() && (wordPart(chars[end].value) || chars[end].value == U'_')) ++end;
    return {end - start, TokenType::Special};
}

RuleMatch number(const RuleContext& context, std::size_t start) {
    return {numberEnd(context.characters, start) - start, TokenType::Number};
}

RuleMatch compoundWord(const RuleContext& context, std::size_t start) {
    const auto& chars = context.characters;
    if (classify(chars[start].value) != TokenType::Word) return {};
    auto end = start + 1;
    bool compound = false;
    while (end < chars.size()) {
        if (wordPart(chars[end].value)) ++end;
        else if (connector(chars[end].value) && end + 1 < chars.size() && wordPart(chars[end + 1].value)) {
            compound = true;
            end += 2;
        } else break;
    }
    return compound ? RuleMatch{end - start, TokenType::Word} : RuleMatch{};
}

RuleMatch word(const RuleContext& context, std::size_t start) {
    const auto& chars = context.characters;
    const auto type = classify(chars[start].value);
    if (type != TokenType::Word && type != TokenType::Number) return {};
    auto end = start + 1;
    while (end < chars.size() && (type == TokenType::Word
        ? wordPart(chars[end].value) : classify(chars[end].value) == TokenType::Number)) ++end;
    return {end - start, type};
}
}
