#include "RuleBasedTokenizer.h"
#include "RuleDetection.h"

std::vector<Token> RuleBasedTokenizer::tokenize(const std::string& text) const {
    const auto characters = tokenization_detail::decodeUtf8(text);
    const RuleContext context{text, characters, {}};
    const auto& rules = RuleDetection::rules();
    tokenization_detail::TokenBuilder builder;

    for (std::size_t index = 0; index < characters.size();) {
        const auto type = tokenization_detail::classify(characters[index].value);
        if (type == TokenType::Whitespace) {
            ++index;
            continue;
        }
        auto match = RuleDetection::matchFirst(context, index, rules);
        if (match.length == 0) {
            // Unmatched symbols still produce a token and guarantee progress.
            match = {1, type};
        }
        const auto& last = characters[index + match.length - 1];
        const auto offset = characters[index].offset;
        const bool sentenceEnd = match.type == TokenType::Punctuation &&
            tokenization_detail::endsSentence(last.value);
        builder.append(text.substr(offset, last.offset + last.length - offset), match.type, sentenceEnd);
        index += match.length;
    }
    return builder.takeTokens();
}
