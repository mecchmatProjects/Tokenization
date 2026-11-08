package tokenization.io;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import tokenization.core.Document;
import tokenization.pipeline.Dataset;
import tokenization.pipeline.TextProcessor;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class CsvExporterTest {

    @TempDir
    Path tempDir;

    private Dataset dataset() {
        TextProcessor processor = new TextProcessor();
        processor.setMaxLength(5);
        return processor.process(List.of(new Document(1, "a", "Machine learning is great!")));
    }

    @Test
    void exportsTokensInReadmeFormat() throws IOException {
        Path file = tempDir.resolve("tokens.csv");
        new CsvExporter().exportTokens(dataset().getDocuments(), file);

        List<String> lines = Files.readAllLines(file, StandardCharsets.UTF_8);
        assertEquals("document_id,position,token,type", lines.get(0));
        assertEquals("1,0,machine,WORD", lines.get(1));
        assertEquals("1,4,!,PUNCTUATION", lines.get(5));
    }

    @Test
    void exportsVocabularyWithSpecialTokens() throws IOException {
        Path file = tempDir.resolve("vocabulary.csv");
        new CsvExporter().exportVocabulary(dataset().getVocabulary(), file);

        List<String> lines = Files.readAllLines(file, StandardCharsets.UTF_8);
        assertEquals("id,token,frequency", lines.get(0));
        assertEquals("0,<PAD>,0", lines.get(1));
        assertEquals("1,<UNK>,0", lines.get(2));
    }

    @Test
    void exportsPaddedSequences() throws IOException {
        Dataset dataset = dataset();
        Path file = tempDir.resolve("encoded.csv");
        new CsvExporter().exportEncoded(dataset.getDocuments(), file, true);

        List<String> lines = Files.readAllLines(file, StandardCharsets.UTF_8);
        assertEquals("document_id,tokens", lines.get(0));
        assertEquals(5, lines.get(1).split(",")[1].replace("\"", "").split(" ").length);
    }

    @Test
    void exportsAllFiles() throws IOException {
        Path directory = tempDir.resolve("out");
        new CsvExporter().exportAll(dataset(), directory);

        assertTrue(Files.exists(directory.resolve("tokens.csv")));
        assertTrue(Files.exists(directory.resolve("vocabulary.csv")));
        assertTrue(Files.exists(directory.resolve("encoded.csv")));
        assertTrue(Files.exists(directory.resolve("bag_of_words.csv")));
    }

    @Test
    void escapesSpecialValues() {
        assertEquals("word", CsvExporter.escape("word"));
        assertEquals("\",\"", CsvExporter.escape(","));
        assertEquals("\"\"\"\"", CsvExporter.escape("\""));
    }
}
