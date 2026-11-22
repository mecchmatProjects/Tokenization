#ifndef LITCRITIC_STYLE_PROFILER_H
#define LITCRITIC_STYLE_PROFILER_H

#include <cstddef>

#include "lexical_analyzer.h"
#include "ngram_analyzer.h"
#include "sentence_analyzer.h"
#include "style_profile.h"
#include "text_document.h"

namespace litcritic {

    struct ProfilerConfig {
        std::size_t topWordsLimit = 10;
        std::size_t topNGramsLimit = 10;
        std::size_t minPhraseRepetitions = 2;
    };

    class StyleProfiler {
    public:
        // Збирає повний стилістичний профіль документа із заданими параметрами вибірки
        static StyleProfile buildProfile(
            const TextDocument& document,
            const ProfilerConfig& config = ProfilerConfig{}
        );
    };

} // namespace litcritic

#endif // LITCRITIC_STYLE_PROFILER_H