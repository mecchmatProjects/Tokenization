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
 * Завантаження текстових документів з диска (README §3, README_LITCRITIC §3).
 *
 * На початку файлу можуть бути рядки з метаданими:
 * <pre>
 * #title: Назва твору
 * #author: Автор
 * #year: 1900
 * #language: uk
 * </pre>
 * Вони не входять до тексту документа.
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

        String[] lines = content.split("\n", -1);
        int bodyStart = 0;
        String title = "";
        String author = "";
        String year = "";
        String language = "";
        while (bodyStart < lines.length && isMetadataLine(lines[bodyStart])) {
            String line = lines[bodyStart].substring(1);
            int colon = line.indexOf(':');
            String key = line.substring(0, colon).trim().toLowerCase();
            String value = line.substring(colon + 1).trim();
            if (key.equals("title")) {
                title = value;
            } else if (key.equals("author")) {
                author = value;
            } else if (key.equals("year")) {
                year = value;
            } else if (key.equals("language")) {
                language = value;
            }
            bodyStart++;
        }

        StringBuilder body = new StringBuilder();
        for (int i = bodyStart; i < lines.length; i++) {
            if (i > bodyStart) {
                body.append('\n');
            }
            body.append(lines[i]);
        }

        Document document = new Document(nextId++, file.getFileName().toString(), body.toString());
        document.setTitle(title);
        document.setAuthor(author);
        document.setLanguage(language);
        try {
            document.setYear(year.isEmpty() ? 0 : Integer.parseInt(year));
        } catch (NumberFormatException e) {
            document.setYear(0);
        }
        return document;
    }

    private boolean isMetadataLine(String line) {
        if (!line.startsWith("#") || !line.contains(":")) {
            return false;
        }
        String key = line.substring(1, line.indexOf(':')).trim().toLowerCase();
        return key.equals("title") || key.equals("author") || key.equals("year") || key.equals("language");
    }

    /**
     * Завантажує всі .txt файли каталогу та його підкаталогів, відсортовані за шляхом.
     * Для структури texts/автор/твір.txt автором вважається назва підкаталогу,
     * якщо його не вказано в самому файлі.
     */
    public List<Document> loadDirectory(Path directory) throws IOException {
        if (!Files.isDirectory(directory)) {
            throw new IOException("Not a directory: " + directory);
        }

        List<Path> files = new ArrayList<>();
        collectFiles(directory, files);
        Collections.sort(files);

        List<Document> documents = new ArrayList<>();
        for (Path file : files) {
            Document document = loadFile(file);
            if (document.getAuthor().isEmpty() && !file.getParent().equals(directory)) {
                document.setAuthor(file.getParent().getFileName().toString());
            }
            documents.add(document);
        }
        return documents;
    }

    private void collectFiles(Path directory, List<Path> files) throws IOException {
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory)) {
            for (Path path : stream) {
                if (Files.isDirectory(path)) {
                    collectFiles(path, files);
                } else if (Files.isRegularFile(path) && path.toString().endsWith(".txt")) {
                    files.add(path);
                }
            }
        }
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
