#pragma once

#include <memory>
#include "TokenizerStrategy.h"
#include "TokenizerType.h"

class TokenizerFactory {
public:
    static std::unique_ptr<TokenizerStrategy> create(TokenizerType type);
};
