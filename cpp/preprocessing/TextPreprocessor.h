#pragma once

#include "Preprocessor.h"
#include "PreprocessingConfig.h"

class TextPreprocessor final : public Preprocessor {
public:
    explicit TextPreprocessor(PreprocessingConfig config = {});
    std::string process(const std::string& text) const override;

private:
    PreprocessingConfig config_;
};
