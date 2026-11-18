#pragma once

#include "TokenizerUtils.h"
#include <functional>
#include <unordered_map>

// Positions and lengths refer to decoded Unicode characters, not UTF-8 bytes.
struct RuleMatch {
    std::size_t length = 0; // Zero means that the rule did not match.
    TokenType type = TokenType::Unknown;
};

struct RuleContext {
    const std::string& text;
    const std::vector<tokenization_detail::Character>& characters;
    // Per-document indexes, built lazily by rules that need them.
    mutable std::unordered_map<std::string, std::vector<std::size_t>> spanCache;
};

using DetectionRule = std::function<RuleMatch(const RuleContext&, std::size_t)>;
