#pragma once
#include <filesystem>
#include <string>
#include <vector>

enum class TextEncoding { Utf8, Utf16LE, Utf16BE, Windows1251 };

struct LoadedText {
    std::string filePath; // UTF-8 path
    std::string text;     // Validated UTF-8 without a leading BOM
};

class TextLoader {
public:
    static std::string decode(const std::string& bytes, TextEncoding encoding = TextEncoding::Utf8);
    static LoadedText loadFile(const std::filesystem::path& path,
                               TextEncoding encoding = TextEncoding::Utf8);
    // Nonrecursive, sorted by path. Throws on any failed file; never returns a partial corpus.
    static std::vector<LoadedText> loadDirectory(const std::filesystem::path& path,
                                                TextEncoding encoding = TextEncoding::Utf8,
                                                const std::string& extension = ".txt");
};
