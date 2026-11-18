#pragma once

#include <string>

class Preprocessor {
public:
    virtual ~Preprocessor() = default;
    virtual std::string process(const std::string& text) const = 0;
};
