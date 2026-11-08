#include "TokenizerFactory.h"
#include "MockTokenizer.h"
#include <stdexcept>

using std::invalid_argument;
using std::make_unique;
using std::unique_ptr;

unique_ptr<TokenizerStrategy> TokenizerFactory::create(TokenizerType type) {
    switch (type) {
        case TokenizerType::Whitespace:
        case TokenizerType::WordPunctuation:
        case TokenizerType::RuleBased:
            // TODO: Restore strategy selection once RuleBased is implemented.
            return make_unique<MockTokenizer>();
        default:
            throw invalid_argument("Unknown tokenizer type");
    }
}
