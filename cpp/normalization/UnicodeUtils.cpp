#include "UnicodeUtils.h"
#include <utf8proc.h>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>

std::string normalization_detail::normalizeUnicode(const std::string& text,
                                                  UnicodeNormalization form,
                                                  bool caseFold) {
    int flags = UTF8PROC_STABLE;
    switch (form) {
        case UnicodeNormalization::None: break;
        case UnicodeNormalization::NFC: flags |= UTF8PROC_COMPOSE; break;
        case UnicodeNormalization::NFKC: flags |= UTF8PROC_COMPOSE | UTF8PROC_COMPAT; break;
        default: throw std::invalid_argument("Unknown Unicode normalization form");
    }
    if (form == UnicodeNormalization::None && !caseFold) return text;
    if (caseFold) flags |= UTF8PROC_CASEFOLD;
    if (text.size() > static_cast<std::size_t>((std::numeric_limits<utf8proc_ssize_t>::max)())) {
        throw std::length_error("Text is too large for Unicode normalization");
    }
    utf8proc_uint8_t* buffer = nullptr;
    const auto length = utf8proc_map(reinterpret_cast<const utf8proc_uint8_t*>(text.data()),
        static_cast<utf8proc_ssize_t>(text.size()), &buffer, static_cast<utf8proc_option_t>(flags));
    const std::unique_ptr<utf8proc_uint8_t, decltype(&std::free)> result(buffer, &std::free);
    if (length < 0) throw std::invalid_argument(utf8proc_errmsg(length));
    return length == 0 ? std::string{} : std::string(reinterpret_cast<const char*>(buffer),
                                                   static_cast<std::size_t>(length));
}
