#pragma once

#include "TokenizerStrategy.h"

class WhitespaceTokenizer final : public TokenizerStrategy {
public:
    std::vector<Token> tokenize(const std::string& text) const override;
};
