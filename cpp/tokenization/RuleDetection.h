#pragma once

#include "TokenRule.h"

namespace RuleDetection {
const std::vector<DetectionRule>& rules();

RuleMatch matchFirst(const RuleContext& context, std::size_t start,
                    const std::vector<DetectionRule>& detectors);
}
