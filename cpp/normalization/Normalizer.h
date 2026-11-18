#pragma once

#include "../include/Token.h"
#include "NormalizationConfig.h"
#include "StopWordDictionary.h"
#include <vector>

class Normalizer {
public:
    std::vector<Token> normalize(
        const std::vector<Token>& tokens,
        const NormalizationConfig& config,
        const StopWordDictionary& stopWords
    ) const;
};
