#pragma once

enum class UnicodeNormalization {
    None,
    NFC,
    NFKC
};

struct NormalizationConfig {
    UnicodeNormalization unicode = UnicodeNormalization::None;
    bool removePunctuation = false;
    bool removeStopWords = false;
    bool replaceNumbers = false;
    bool replaceUrls = false;
    bool replaceEmails = false;
    bool stopWordsCaseSensitive = false;
};
