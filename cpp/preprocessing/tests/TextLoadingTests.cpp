#include "preprocessing/TextLoader.h"
#include "Application.h"
#include "MainWindow.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class F> void rejects(F action, const char* message) {
    bool failed = false;
    try { action(); } catch (const std::exception&) { failed = true; }
    require(failed, message);
}
void write(const fs::path& path, const std::string& bytes) {
    std::ofstream file(path, std::ios::binary);
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (!file) throw std::runtime_error("Cannot create test fixture");
}
struct Workspace {
    fs::path path = fs::temp_directory_path() / fs::u8path(u8"тексти-тест-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Workspace() { fs::create_directories(path); }
    ~Workspace() { std::error_code error; fs::remove_all(path, error); }
};
}

int main() {
    try {
        using E = TextEncoding;
        require(TextLoader::decode("").empty(), "Empty UTF-8 file");
        require(TextLoader::decode(u8"Привіт 你好 😀") == u8"Привіт 你好 😀", "UTF-8 preservation");
        require(TextLoader::decode("\xEF\xBB\xBFhello") == "hello", "UTF-8 BOM removal");
        const std::string le("\xFF\xFE\x16\x04\x3D\xD8\0\xDE", 8);
        const std::string be("\xFE\xFF\x04\x16\xD8\x3D\xDE\0", 8);
        require(TextLoader::decode(le, E::Utf16LE) == u8"Ж😀", "UTF-16 LE and surrogate pair");
        require(TextLoader::decode(be, E::Utf16BE) == u8"Ж😀", "UTF-16 BE and surrogate pair");
        require(TextLoader::decode(le.substr(2), E::Utf16LE) == u8"Ж😀", "BOM-less UTF-16");
        const std::string cp = "\xCF\xF0\xE8\xE2\xB3\xF2 \xA5\xAA\xAF";
        require(TextLoader::decode(cp, E::Windows1251) == u8"Привіт ҐЄЇ", "Ukrainian Windows-1251");
        rejects([&] { TextLoader::decode(cp); }, "Wrong UTF-8 encoding must fail");
        rejects([&] { TextLoader::decode(le, E::Utf8); }, "Conflicting BOM must fail");
        rejects([&] { TextLoader::decode("\xEF\xBB\xBF", E::Windows1251); }, "UTF-8 BOM conflicts with CP1251");
        rejects([&] { TextLoader::decode(std::string("\xFF\xFE\0\0", 4), E::Utf16LE); }, "UTF-32 must fail");
        rejects([&] { TextLoader::decode("\xFF"); }, "Invalid UTF-8");
        rejects([&] { TextLoader::decode("\xC3"); }, "Truncated UTF-8");
        rejects([&] { TextLoader::decode(std::string("a\0b", 3)); }, "NUL cannot enter the GUI");
        rejects([&] { TextLoader::decode("\x16", E::Utf16LE); }, "Odd UTF-16 byte count");
        rejects([&] { TextLoader::decode(std::string("\0\xD8", 2), E::Utf16LE); }, "Missing low surrogate");
        rejects([&] { TextLoader::decode(std::string("\0\xDC", 2), E::Utf16LE); }, "Unpaired low surrogate");
        rejects([&] { TextLoader::decode(std::string("\0\xD8" "A\0", 4), E::Utf16LE); }, "Invalid surrogate pair");
        rejects([&] { TextLoader::decode("\x98", E::Windows1251); }, "Undefined Windows-1251 byte");
        rejects([&] { TextLoader::decode("", static_cast<E>(99)); }, "Unknown encoding");

        Workspace temp;
        const auto corpus = temp.path / "corpus";
        fs::create_directory(corpus);
        const auto first = corpus / fs::u8path(u8"1-привіт.txt");
        const auto second = corpus / "2.TXT";
        write(second, cp);
        write(first, cp);
        write(corpus / "ignored.csv", "\xFF");
        fs::create_directory(corpus / "subdirectory");
        write(corpus / "subdirectory" / "ignored.txt", "\xFF");
        const auto files = TextLoader::loadDirectory(corpus, E::Windows1251);
        require(files.size() == 2 && files[0].filePath == first.u8string() &&
                files[1].filePath == second.u8string(), "Sorted nonrecursive .txt import with Unicode paths");
        require(files[0].text == u8"Привіт ҐЄЇ", "Directory decoding");
        rejects([&] { TextLoader::loadFile(temp.path / "missing.txt"); }, "Missing file");

        Dataset dataset;
        require(dataset.loadDocumentFromFile(first.u8string(), false, nullptr, E::Windows1251), "Dataset file API");
        require(dataset.getDocuments()[0].rawText == files[0].text, "Dataset must store UTF-8");
        require(dataset.loadFromDirectory(corpus.u8string(), ".txt", false, nullptr, E::Windows1251) == 2,
                "Dataset directory API");
        require(dataset.getDocuments()[2].id == 3, "Document IDs when appending");

        GuiConfig guiConfig;
        guiConfig.inputEncoding = E::Windows1251;
        MainWindow gui(guiConfig); // Test input and pipeline state without creating native windows.
        gui.loadTextFile(first.u8string());
        require(gui.getCurrentDataset().size() == 1, "GUI must load one document");
        require(gui.getCurrentDataset().getDocuments()[0].tokens[0].text == u8"привіт", "Imported text uses NLP pipeline");
        gui.runFullPipeline();
        require(gui.getCurrentDataset().size() == 1 && gui.getCurrentVocabulary().contains(u8"привіт"),
                "Full pipeline must process imported file");
        const auto bad = corpus / "3.txt";
        write(bad, "\x98");
        rejects([&] { gui.loadTextFile(bad.u8string()); }, "GUI must reject an invalid file");
        require(gui.getCurrentDataset().size() == 1 && gui.getCurrentStage() == PipelineStage::Encoded,
                "Failed import must preserve existing pipeline results");
        rejects([&] { dataset.loadFromDirectory(corpus.u8string(), ".txt", false, nullptr, E::Windows1251); },
                "Dataset must reject invalid corpus");
        require(dataset.size() == 3, "Dataset must not append a partial corpus");
        gui.loadTextFile(second.u8string());
        require(gui.getCurrentDataset().size() == 1 && gui.getCurrentDataset().getDocuments()[0].filePath == second.u8string(),
                "Opening another file must replace previous input");
        auto config = gui.getConfig();
        config.inputText = "Manual input";
        gui.setConfig(config);
        gui.runTokenizationStep();
        require(gui.getCurrentDataset().size() == 1 && gui.getCurrentDataset().getDocuments()[0].rawText == "Manual input",
                "Manual input must replace imported input");

        fs::remove(bad);
        PipelineConfig pipeline;
        pipeline.textEncoding = E::Windows1251;
        Application application(pipeline);
        application.runOnDirectory(corpus.u8string());
        require(application.getDataset().size() == 2, "Application must forward the selected encoding");
        const auto emptyFile = temp.path / "empty.txt";
        write(emptyFile, "");
        require(TextLoader::loadFile(emptyFile).text.empty(), "Empty file must be accepted");
        std::cout << "Text loading checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
