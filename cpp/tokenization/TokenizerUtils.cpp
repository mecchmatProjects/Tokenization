#include "TokenizerUtils.h"
#include <utf8proc.h>
#include <stdexcept>
#include <utility>

using std::invalid_argument;
using std::move;
using std::size_t;
using std::string;
using std::vector;

namespace tokenization_detail {

vector<Character> decodeUtf8(const string& text) {
    vector<Character> characters;
    for (size_t offset = 0; offset < text.size();) {
        utf8proc_int32_t value = 0;
        const utf8proc_ssize_t length = utf8proc_iterate(
            reinterpret_cast<const utf8proc_uint8_t*>(text.data() + offset),
            static_cast<utf8proc_ssize_t>((text.size() - offset < 4) ? text.size() - offset : 4),
            &value);
        if (length < 0) {
            throw invalid_argument(utf8proc_errmsg(length));
        }

        characters.push_back({static_cast<char32_t>(value), offset, static_cast<size_t>(length)});
        offset += static_cast<size_t>(length);
    }
    return characters;
}

TokenType classify(char32_t character) {
    if ((character >= U'\t' && character <= U'\r') || character == U'\u0085') {
        return TokenType::Whitespace;
    }
    if (character == U'\u02BC') {
        return TokenType::Punctuation;
    }
    const string special = "@#$%^&*+=<>|~`";
    if (character < 0x80 && special.find(static_cast<char>(character)) != string::npos) {
        return TokenType::Special;
    }

    switch (utf8proc_category(static_cast<utf8proc_int32_t>(character))) {
        case UTF8PROC_CATEGORY_LU:
        case UTF8PROC_CATEGORY_LL:
        case UTF8PROC_CATEGORY_LT:
        case UTF8PROC_CATEGORY_LM:
        case UTF8PROC_CATEGORY_LO:
        case UTF8PROC_CATEGORY_MN:
        case UTF8PROC_CATEGORY_MC:
        case UTF8PROC_CATEGORY_ME:
            return TokenType::Word;
        case UTF8PROC_CATEGORY_ND:
        case UTF8PROC_CATEGORY_NL:
        case UTF8PROC_CATEGORY_NO:
            return TokenType::Number;
        case UTF8PROC_CATEGORY_PC:
        case UTF8PROC_CATEGORY_PD:
        case UTF8PROC_CATEGORY_PS:
        case UTF8PROC_CATEGORY_PE:
        case UTF8PROC_CATEGORY_PI:
        case UTF8PROC_CATEGORY_PF:
        case UTF8PROC_CATEGORY_PO:
            return TokenType::Punctuation;
        case UTF8PROC_CATEGORY_SM:
        case UTF8PROC_CATEGORY_SC:
        case UTF8PROC_CATEGORY_SK:
        case UTF8PROC_CATEGORY_SO:
            return TokenType::Special;
        case UTF8PROC_CATEGORY_ZS:
        case UTF8PROC_CATEGORY_ZL:
        case UTF8PROC_CATEGORY_ZP:
            return TokenType::Whitespace;
        default:
            return TokenType::Unknown;
    }
}

bool endsSentence(char32_t character) {
    return character == U'.' || character == U'!' || character == U'?' || character == 0x2026;
}

void TokenBuilder::append(const string& text, TokenType type, bool sentenceEnd) {
    if (pendingSentenceEnd && type != TokenType::Punctuation) {
        ++sentenceNumber;
        pendingSentenceEnd = false;
    }
    tokens.emplace_back(text, type, tokens.size(), sentenceNumber);
    pendingSentenceEnd = pendingSentenceEnd || sentenceEnd;
}

vector<Token> TokenBuilder::takeTokens() {
    return move(tokens);
}

}
