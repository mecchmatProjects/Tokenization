#include "RuleDetection.h"
#include "RuleDetectors.h"
#include <stdexcept>

namespace RuleDetection {
const std::vector<DetectionRule>& rules() {
    static const std::vector<DetectionRule> detectors = {
        RuleDetectors::url,
        RuleDetectors::email,
        RuleDetectors::predefinedWord,
        RuleDetectors::hashtag,
        RuleDetectors::number,
        RuleDetectors::compoundWord,
        RuleDetectors::word,
    };
    return detectors;
}

RuleMatch matchFirst(const RuleContext& context, std::size_t start,
                    const std::vector<DetectionRule>& detectors) {
    if (start >= context.characters.size()) return {};
    for (const auto& detect : detectors) {
        const auto match = detect(context, start);
        if (match.length > context.characters.size() - start) {
            throw std::logic_error("Token detection rule returned a length beyond the input");
        }
        if (match.length != 0) return match;
    }
    return {};
}
}
