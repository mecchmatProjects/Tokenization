package tokenization.io;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import tokenization.core.Document;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

class TextLoaderTest {

    @TempDir
    Path tempDir;

    @Test
    void loadsSingleFile() throws IOException {
        Path file = tempDir.resolve("a.txt");
        Files.writeString(file, "Hello world", StandardCharsets.UTF_8);

        Document document = new TextLoader().loadFile(file);

        assertEquals("a.txt", document.getName());
        assertEquals("Hello world", document.getContent());
        assertEquals(1, document.getId());
    }

    @Test
    void loadsOnlyTxtFilesFromDirectorySortedByName() throws IOException {
        Files.writeString(tempDir.resolve("b.txt"), "second");
        Files.writeString(tempDir.resolve("a.txt"), "first");
        Files.writeString(tempDir.resolve("notes.md"), "ignored");

        List<Document> documents = new TextLoader().loadDirectory(tempDir);

        assertEquals(2, documents.size());
        assertEquals("first", documents.get(0).getContent());
        assertEquals("second", documents.get(1).getContent());
        assertEquals(2, documents.get(1).getId());
    }

    @Test
    void usesGivenCharset() throws IOException {
        Charset cp1251 = Charset.forName("windows-1251");
        Path file = tempDir.resolve("ua.txt");
        Files.writeString(file, "Привіт світе", cp1251);

        Document document = new TextLoader(cp1251).loadFile(file);

        assertEquals("Привіт світе", document.getContent());
    }

    @Test
    void removesUtf8Bom() throws IOException {
        Path file = tempDir.resolve("bom.txt");
        Files.writeString(file, "﻿text", StandardCharsets.UTF_8);

        assertEquals("text", new TextLoader().loadFile(file).getContent());
    }

    @Test
    void failsForMissingDirectory() {
        assertThrows(IOException.class, () -> new TextLoader().loadDirectory(tempDir.resolve("missing")));
    }
}
