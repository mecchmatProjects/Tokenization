#pragma once

enum class NewlineMode {
    Preserve,
    NormalizeToLf,
    ReplaceWithSpace
};

struct PreprocessingConfig {
    bool collapseHorizontalWhitespace = true;
    bool trimWhitespace = true;
    bool lowercase = false;
    NewlineMode newlineMode = NewlineMode::NormalizeToLf;
};
