#include "TextLoader.h"
#include <utf8proc.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
void append(std::string& output, utf8proc_int32_t value) {
    if (value == 0) throw std::invalid_argument("NUL character in text; check the selected encoding");
    utf8proc_uint8_t buffer[4];
    const auto size = utf8proc_encode_char(value, buffer);
    output.append(reinterpret_cast<const char*>(buffer), static_cast<std::size_t>(size));
}
bool prefix(const std::string& bytes, const std::string& mark) {
    return bytes.compare(0, mark.size(), mark) == 0;
}
std::string lowerExtension(std::string extension) {
    for (auto& ch : extension) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    return extension;
}
}

std::string TextLoader::decode(const std::string& bytes, TextEncoding encoding) {
    switch (encoding) {
        case TextEncoding::Utf8: case TextEncoding::Utf16LE:
        case TextEncoding::Utf16BE: case TextEncoding::Windows1251: break;
        default: throw std::invalid_argument("Unknown text encoding");
    }
    if (prefix(bytes, std::string("\xFF\xFE\0\0", 4)) ||
        prefix(bytes, std::string("\0\0\xFE\xFF", 4))) {
        throw std::invalid_argument("UTF-32 is not supported; convert the file to UTF-8");
    }
    std::size_t offset = 0;
    if (prefix(bytes, "\xEF\xBB\xBF")) {
        if (encoding != TextEncoding::Utf8) throw std::invalid_argument("UTF-8 BOM conflicts with the selected encoding");
        offset = 3;
    } else if (prefix(bytes, "\xFF\xFE")) {
        if (encoding != TextEncoding::Utf16LE) throw std::invalid_argument("UTF-16 LE BOM conflicts with the selected encoding");
        offset = 2;
    } else if (prefix(bytes, "\xFE\xFF")) {
        if (encoding != TextEncoding::Utf16BE) throw std::invalid_argument("UTF-16 BE BOM conflicts with the selected encoding");
        offset = 2;
    }
    std::string output;
    if (encoding == TextEncoding::Utf8) {
        for (auto i = offset; i < bytes.size();) {
            utf8proc_int32_t value;
            const auto count = utf8proc_iterate(
                reinterpret_cast<const utf8proc_uint8_t*>(bytes.data() + i),
                static_cast<utf8proc_ssize_t>(std::min<std::size_t>(4, bytes.size() - i)), &value);
            if (count < 0) throw std::invalid_argument("Invalid UTF-8 at byte " + std::to_string(i) + "; check the selected encoding");
            append(output, value);
            i += static_cast<std::size_t>(count);
        }
    } else if (encoding == TextEncoding::Utf16LE || encoding == TextEncoding::Utf16BE) {
        if ((bytes.size() - offset) % 2) throw std::invalid_argument("Incomplete UTF-16 code unit");
        auto unit = [&](std::size_t i) -> unsigned {
            const auto a = static_cast<unsigned char>(bytes[i]);
            const auto b = static_cast<unsigned char>(bytes[i + 1]);
            return encoding == TextEncoding::Utf16LE ? a | (b << 8) : (a << 8) | b;
        };
        while (offset < bytes.size()) {
            auto value = unit(offset);
            offset += 2;
            if (value >= 0xD800 && value <= 0xDBFF) {
                if (offset == bytes.size()) throw std::invalid_argument("Incomplete UTF-16 surrogate pair");
                const auto low = unit(offset);
                if (low < 0xDC00 || low > 0xDFFF) throw std::invalid_argument("Invalid UTF-16 surrogate pair");
                value = 0x10000 + ((value - 0xD800) << 10) + low - 0xDC00;
                offset += 2;
            } else if (value >= 0xDC00 && value <= 0xDFFF) {
                throw std::invalid_argument("Unpaired UTF-16 low surrogate");
            }
            append(output, static_cast<utf8proc_int32_t>(value));
        }
    } else {
        static const unsigned short table[] = {
            0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,
            0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
            0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
            0,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
            0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,
            0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
            0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,
            0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457
        };
        for (unsigned char byte : bytes) {
            if (byte == 0x98) throw std::invalid_argument("Undefined Windows-1251 byte 0x98");
            append(output, byte < 0x80 ? byte : byte >= 0xC0 ? 0x0410 + byte - 0xC0 : table[byte - 0x80]);
        }
    }
    return output;
}

LoadedText TextLoader::loadFile(const std::filesystem::path& path, TextEncoding encoding) {
    try {
        if (!std::filesystem::is_regular_file(path)) throw std::runtime_error("Not a readable regular file");
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open file");
        std::ostringstream buffer;
        buffer << file.rdbuf();
        if (file.bad()) throw std::runtime_error("Cannot read file");
        return {path.u8string(), decode(buffer.str(), encoding)};
    } catch (const std::exception& error) {
        throw std::runtime_error(path.u8string() + ": " + error.what());
    }
}

std::vector<LoadedText> TextLoader::loadDirectory(const std::filesystem::path& path,
                                                TextEncoding encoding, const std::string& extension) {
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file() && (extension.empty() ||
            lowerExtension(entry.path().extension().u8string()) == lowerExtension(extension))) {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    std::vector<LoadedText> result;
    for (const auto& file : files) result.push_back(loadFile(file, encoding));
    return result;
}
