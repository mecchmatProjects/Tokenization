package tokenization.io;

import tokenization.core.Document;

import java.nio.file.Path;
import java.util.List;

/**
 * Завантаження текстових документів з диска (README §3).
 */
public class TextLoader {

    public Document loadFile(Path file) {
        // TODO: прочитати .txt файл із заданим кодуванням
        throw new UnsupportedOperationException("Not implemented yet");
    }

    public List<Document> loadDirectory(Path directory) {
        // TODO: завантажити всі .txt файли каталогу
        throw new UnsupportedOperationException("Not implemented yet");
    }
}
