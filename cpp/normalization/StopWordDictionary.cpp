#include "StopWordDictionary.h"
#include "tokenization/TokenizerUtils.h"
#include <fstream>
#include <stdexcept>

void StopWordDictionary::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open stop-word file: " + path);
    std::unordered_set<std::string> loaded;
    std::string line;
    bool first = true;
    while (std::getline(file, line)) {
        if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
        first = false;
        const auto chars = tokenization_detail::decodeUtf8(line);
        std::size_t begin = 0;
        auto end = chars.size();
        while (begin < end && tokenization_detail::classify(chars[begin].value) == TokenType::Whitespace) ++begin;
        while (end > begin && tokenization_detail::classify(chars[end - 1].value) == TokenType::Whitespace) --end;
        if (begin == end) continue;
        for (auto i = begin; i < end; ++i) {
            if (tokenization_detail::classify(chars[i].value) == TokenType::Whitespace) {
                throw std::invalid_argument("Expected one stop word per line in: " + path);
            }
        }
        const auto offset = chars[begin].offset;
        loaded.insert(line.substr(offset, chars[end - 1].offset + chars[end - 1].length - offset));
    }
    if (file.bad()) throw std::runtime_error("Cannot read stop-word file: " + path);
    words_.swap(loaded);
}

void StopWordDictionary::add(const std::string& word) {
    tokenization_detail::decodeUtf8(word);
    if (!word.empty()) words_.insert(word);
}

const std::unordered_set<std::string>& StopWordDictionary::words() const {
    return words_;
}

bool StopWordDictionary::contains(const std::string& word) const {
    return words_.find(word) != words_.end();
}
