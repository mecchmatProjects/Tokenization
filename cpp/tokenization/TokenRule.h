#pragma once

#include "TokenizerUtils.h"
#include <functional>
#include <unordered_map>

struct RuleMatch {
    std::size_t length = 0;
    TokenType type = TokenType::Unknown;
};

struct RuleContext {
    const std::string& text;
    const std::vector<tokenization_detail::Character>& characters;
    mutable std::unordered_map<std::string, std::vector<std::size_t>> spanCache;
};

using DetectionRule = std::function<RuleMatch(const RuleContext&, std::size_t)>;
