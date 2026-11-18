#include "Normalizer.h"
#include "SpecialMarkers.h"
#include "UnicodeUtils.h"
#include "tokenization/RuleDetectors.h"
#include <stdexcept>
#include <utility>

std::vector<Token> Normalizer::normalize(
    const std::vector<Token>& tokens,
    const NormalizationConfig& config,
    const StopWordDictionary& stopWords
) const {
    using normalization_detail::normalizeUnicode;
    normalizeUnicode("", config.unicode);
    std::unordered_set<std::string> stopKeys;
    if (config.removeStopWords) {
        for (const auto& word : stopWords.words()) {
            stopKeys.insert(normalizeUnicode(word, config.unicode, !config.stopWordsCaseSensitive));
        }
    }
    std::vector<Token> result;
    result.reserve(tokens.size());
    for (const auto& original : tokens) {
        Token token = original;
        token.text = normalizeUnicode(token.text, config.unicode);
        if ((config.replaceUrls || config.replaceEmails) && !token.text.empty()) {
            const auto chars = tokenization_detail::decodeUtf8(token.text);
            const RuleContext context{token.text, chars, {}};
            const auto url = RuleDetectors::url(context, 0);
            if (url.length == chars.size()) {
                if (config.replaceUrls) { token.text = SpecialMarkers::Url; token.type = TokenType::Special; }
            } else if (config.replaceEmails && RuleDetectors::email(context, 0).length == chars.size()) {
                token.text = SpecialMarkers::Email;
                token.type = TokenType::Special;
            }
        }
        if (config.replaceNumbers && token.type == TokenType::Number) {
            token.text = SpecialMarkers::Number;
            token.type = TokenType::Special;
        }
        if (config.removePunctuation && token.type == TokenType::Punctuation) continue;
        if (config.removeStopWords && token.type == TokenType::Word &&
            stopKeys.count(normalizeUnicode(token.text, config.unicode, !config.stopWordsCaseSensitive))) continue;
        if (token.text != original.text || token.type != original.type) token.tokenId = -1;
        result.push_back(std::move(token));
    }
    return result;
}
