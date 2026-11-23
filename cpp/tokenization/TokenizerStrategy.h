#pragma once

#include <string>
#include <vector>
#include "../include/Token.h"

class TokenizerStrategy {
public:
    virtual ~TokenizerStrategy() = default;

    virtual std::vector<Token> tokenize(const std::string& text) const = 0;
};
