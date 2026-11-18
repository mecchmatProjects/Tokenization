#pragma once

#include "NormalizationConfig.h"
#include <string>

namespace normalization_detail {
std::string normalizeUnicode(const std::string& text, UnicodeNormalization form,
                             bool caseFold = false);
}
