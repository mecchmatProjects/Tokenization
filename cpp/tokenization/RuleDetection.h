#pragma once

#include "TokenRule.h"

namespace RuleDetection {
// Edit the ordered function list in RuleDetection.cpp, then rebuild.
const std::vector<DetectionRule>& rules();

// First matching function wins. Invalid match lengths throw std::logic_error.
RuleMatch matchFirst(const RuleContext& context, std::size_t start,
                    const std::vector<DetectionRule>& detectors);
}
