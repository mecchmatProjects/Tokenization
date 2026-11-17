#pragma once

#include "TokenRule.h"

namespace RuleDetectors {
RuleMatch url(const RuleContext& context, std::size_t start);
RuleMatch email(const RuleContext& context, std::size_t start);
RuleMatch predefinedWord(const RuleContext& context, std::size_t start);
RuleMatch hashtag(const RuleContext& context, std::size_t start);
RuleMatch number(const RuleContext& context, std::size_t start);
RuleMatch compoundWord(const RuleContext& context, std::size_t start);
RuleMatch word(const RuleContext& context, std::size_t start);
}
