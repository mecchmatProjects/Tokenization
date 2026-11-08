#include "WhitespaceTokenizer.h"
#include "TokenizerUtils.h"

using std::size_t;
using std::string;
using std::vector;
using tokenization_detail::Character;
using tokenization_detail::classify;
using tokenization_detail::decodeUtf8;
using tokenization_detail::endsSentence;
using tokenization_detail::TokenBuilder;

vector<Token> WhitespaceTokenizer::tokenize(const string& text) const {
    const vector<Character> characters = decodeUtf8(text);
    TokenBuilder builder;
    size_t index = 0;

    while (index < characters.size()) {
        if (classify(characters[index].value) == TokenType::Whitespace) {
            ++index;
            continue;
        }

        const size_t start = index;
        TokenType type = classify(characters[index].value);
        while (index < characters.size() &&
               classify(characters[index].value) != TokenType::Whitespace) {
            if (classify(characters[index].value) != type) {
                type = TokenType::Word;
            }
            ++index;
        }

        const Character& last = characters[index - 1];
        const size_t offset = characters[start].offset;
        builder.append(text.substr(offset, last.offset + last.length - offset),
                       type, endsSentence(last.value));
    }
    return builder.takeTokens();
}
