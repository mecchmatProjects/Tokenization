#pragma once

#include <string>
#include <unordered_set>

class StopWordDictionary {
public:
    void loadFromFile(const std::string& path);
    void add(const std::string& word);
    bool contains(const std::string& word) const;
    const std::unordered_set<std::string>& words() const;

private:
    std::unordered_set<std::string> words_;
};
