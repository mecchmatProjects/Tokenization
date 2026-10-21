package tokenization.io;

import tokenization.core.Document;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Завантаження текстових документів з диска (README §3).
 */
public class TextLoader {

    private final Charset charset;
    // документи нумеруються з 1 у порядку завантаження
    private int nextId = 1;

    public TextLoader() {
        this(StandardCharsets.UTF_8);
    }

    public TextLoader(Charset charset) {
        this.charset = charset;
    }

    public Charset getCharset() {
        return charset;
    }

    public Document loadFile(Path file) throws IOException {
        String content = Files.readString(file, charset);
        content = removeBom(content);
        String name = file.getFileName().toString();
        return new Document(nextId++, name, content);
    }

    /**
     * Завантажує всі .txt файли каталогу (без підкаталогів), відсортовані за назвою.
     */
    public List<Document> loadDirectory(Path directory) throws IOException {
        if (!Files.isDirectory(directory)) {
            throw new IOException("Not a directory: " + directory);
        }

        List<Path> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory, "*.txt")) {
            for (Path path : stream) {
                if (Files.isRegularFile(path)) {
                    files.add(path);
                }
            }
        }
        Collections.sort(files);

        List<Document> documents = new ArrayList<>();
        for (Path file : files) {
            documents.add(loadFile(file));
        }
        return documents;
    }

    /**
     * Завантажує файл або каталог залежно від того, що передано.
     */
    public List<Document> load(Path path) throws IOException {
        if (Files.isDirectory(path)) {
            return loadDirectory(path);
        }
        List<Document> documents = new ArrayList<>();
        documents.add(loadFile(path));
        return documents;
    }

    // Деякі редактори додають BOM на початок UTF-8 файлу, він не є частиною тексту
    private String removeBom(String content) {
        if (!content.isEmpty() && content.charAt(0) == '﻿') {
            return content.substring(1);
        }
        return content;
    }
}
