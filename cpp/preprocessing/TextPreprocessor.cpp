#include "TextPreprocessor.h"

#include <utf8proc.h>
#include <stdexcept>

namespace {

bool isNewline(utf8proc_int32_t value) {
    return value == '\r' || value == '\n' || value == '\v' ||
           value == '\f' || value == 0x85 || value == 0x2028 || value == 0x2029;
}

bool isHorizontalWhitespace(utf8proc_int32_t value) {
    return value == '\t' || utf8proc_category(value) == UTF8PROC_CATEGORY_ZS;
}

void appendUtf8(std::string& output, utf8proc_int32_t value) {
    utf8proc_uint8_t bytes[4];
    const auto length = utf8proc_encode_char(value, bytes);
    output.append(reinterpret_cast<const char*>(bytes), static_cast<std::size_t>(length));
}

}

TextPreprocessor::TextPreprocessor(PreprocessingConfig config) : config_(config) {
    switch (config_.newlineMode) {
        case NewlineMode::Preserve:
        case NewlineMode::NormalizeToLf:
        case NewlineMode::ReplaceWithSpace:
            break;
        default:
            throw std::invalid_argument("Unknown newline mode");
    }
}

std::string TextPreprocessor::process(const std::string& text) const {
    std::string output;
    output.reserve(text.size());
    std::size_t lastNonWhitespaceEnd = 0;
    bool previousHorizontalWhitespace = false;

    for (std::size_t offset = 0; offset < text.size();) {
        utf8proc_int32_t value = 0;
        const auto remaining = text.size() - offset;
        const auto length = utf8proc_iterate(
            reinterpret_cast<const utf8proc_uint8_t*>(text.data() + offset),
            static_cast<utf8proc_ssize_t>(remaining < 4 ? remaining : 4), &value);
        if (length < 0) {
            throw std::invalid_argument("Invalid UTF-8 at byte " + std::to_string(offset) +
                                        ": " + utf8proc_errmsg(length));
        }
        offset += static_cast<std::size_t>(length);

        if (isNewline(value) && config_.newlineMode != NewlineMode::Preserve) {
            // Treat CRLF as one logical newline.
            if (value == '\r' && offset < text.size() && text[offset] == '\n') {
                ++offset;
            }
            value = config_.newlineMode == NewlineMode::NormalizeToLf ? '\n' : ' ';
        }

        const bool horizontal = isHorizontalWhitespace(value);
        const bool whitespace = horizontal || isNewline(value);
        if (config_.trimWhitespace && output.empty() && whitespace) {
            continue;
        }
        if (horizontal && config_.collapseHorizontalWhitespace) {
            if (previousHorizontalWhitespace) {
                continue;
            }
            value = ' ';
        }
        previousHorizontalWhitespace = horizontal;
        if (config_.lowercase) {
            value = utf8proc_tolower(value);
        }
        appendUtf8(output, value);
        if (!whitespace) {
            lastNonWhitespaceEnd = output.size();
        }
    }
    if (config_.trimWhitespace) {
        output.resize(lastNonWhitespaceEnd);
    }
    return output;
}
