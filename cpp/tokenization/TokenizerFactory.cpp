#include "TokenizerFactory.h"
#include "MockTokenizer.h"
#include "WhitespaceTokenizer.h"
#include "WordPunctuationTokenizer.h"
#include <stdexcept>

using std::invalid_argument;
using std::make_unique;
using std::unique_ptr;

unique_ptr<TokenizerStrategy> TokenizerFactory::create(TokenizerType type) {
    switch (type) {
        case TokenizerType::Whitespace:
            return make_unique<WhitespaceTokenizer>();
        case TokenizerType::WordPunctuation:
            return make_unique<WordPunctuationTokenizer>();
        case TokenizerType::RuleBased:
            // RuleBased still needs a standalone implementation.
            return make_unique<MockTokenizer>();
        default:
            throw invalid_argument("Unknown tokenizer type");
    }
}
