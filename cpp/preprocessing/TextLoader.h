#pragma once
#include <filesystem>
#include <string>
#include <vector>

enum class TextEncoding { Utf8, Utf16LE, Utf16BE, Windows1251 };

struct LoadedText {
    std::string filePath;
    std::string text;
};

class TextLoader {
public:
    static std::string decode(const std::string& bytes, TextEncoding encoding = TextEncoding::Utf8);
    static LoadedText loadFile(const std::filesystem::path& path,
                               TextEncoding encoding = TextEncoding::Utf8);
    static std::vector<LoadedText> loadDirectory(const std::filesystem::path& path,
                                                TextEncoding encoding = TextEncoding::Utf8,
                                                const std::string& extension = ".txt");
};
