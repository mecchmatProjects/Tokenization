package tokenization.experiment;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.encoding.IntegerEncoder;
import tokenization.io.CsvExporter;
import tokenization.io.TextLoader;
import tokenization.normalization.Normalizer;
import tokenization.pipeline.Dataset;
import tokenization.pipeline.ProcessedDocument;
import tokenization.preprocessing.TextPreprocessor;
import tokenization.tokenizer.RuleBasedTokenizer;
import tokenization.vocabulary.Vocabulary;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * Вимірювання часу етапів pipeline для різних розмірів даних (README §32).
 *
 * Великий текст отримується повторенням наявних документів до потрібного розміру.
 * Результати можна зберегти в CSV і побудувати графік T = f(N).
 */
public class PerformanceExperiment {

    public List<PerformanceResult> run(List<Document> source, int[] sizesKb) throws IOException {
        // прогрів JVM на малому тексті, інакше перший вимір буде помітно повільнішим
        measure(buildText(source, 64 * 1024), 64);

        List<PerformanceResult> results = new ArrayList<>();
        for (int sizeKb : sizesKb) {
            results.add(measure(buildText(source, sizeKb * 1024), sizeKb));
        }
        return results;
    }

    private PerformanceResult measure(String text, int sizeKb) throws IOException {
        PerformanceResult result = new PerformanceResult();
        result.sizeKb = sizeKb;
        result.characters = text.length();

        Path tempDir = Files.createTempDirectory("tokenization-perf");
        Path inputFile = tempDir.resolve("input.txt");
        Files.writeString(inputFile, text, StandardCharsets.UTF_8);

        long start = System.nanoTime();
        Document document = new TextLoader().loadFile(inputFile);
        result.loadMs = elapsedMs(start);

        start = System.nanoTime();
        String processed = new TextPreprocessor().process(document.getContent());
        result.preprocessMs = elapsedMs(start);

        start = System.nanoTime();
        List<Token> tokens = new RuleBasedTokenizer().tokenize(processed);
        List<Token> normalized = new Normalizer().normalize(tokens);
        result.tokenizeMs = elapsedMs(start);
        result.tokens = normalized.size();

        start = System.nanoTime();
        List<String> texts = Normalizer.texts(normalized);
        List<List<String>> documents = new ArrayList<>();
        documents.add(texts);
        Vocabulary vocabulary = Vocabulary.build(documents, 1, 0);
        result.vocabularyMs = elapsedMs(start);

        start = System.nanoTime();
        ProcessedDocument processedDocument = new ProcessedDocument(document, processed, tokens, normalized);
        List<Integer> ids = new IntegerEncoder(vocabulary).encode(texts);
        processedDocument.setEncoded(ids);
        processedDocument.setPadded(IntegerEncoder.pad(ids, 100));
        result.encodeMs = elapsedMs(start);

        start = System.nanoTime();
        List<ProcessedDocument> processedList = new ArrayList<>();
        processedList.add(processedDocument);
        Path outputDir = tempDir.resolve("output");
        new CsvExporter().exportAll(new Dataset(processedList, vocabulary, 100), outputDir);
        result.exportMs = elapsedMs(start);

        deleteTempFiles(tempDir, outputDir);
        return result;
    }

    /** Склеює документи в один текст, повторюючи їх, доки не набереться потрібна кількість символів. */
    static String buildText(List<Document> source, int characters) {
        if (source.isEmpty()) {
            throw new IllegalArgumentException("No source documents");
        }
        StringBuilder sb = new StringBuilder(characters + 1000);
        int index = 0;
        while (sb.length() < characters) {
            sb.append(source.get(index).getContent()).append("\n\n");
            index = (index + 1) % source.size();
        }
        return sb.toString();
    }

    private double elapsedMs(long start) {
        return (System.nanoTime() - start) / 1_000_000.0;
    }

    private void deleteTempFiles(Path tempDir, Path outputDir) throws IOException {
        String[] files = {"tokens.csv", "vocabulary.csv", "encoded.csv", "bag_of_words.csv"};
        for (String file : files) {
            Files.deleteIfExists(outputDir.resolve(file));
        }
        Files.deleteIfExists(outputDir);
        Files.deleteIfExists(tempDir.resolve("input.txt"));
        Files.deleteIfExists(tempDir);
    }

    public String format(List<PerformanceResult> results) {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%8s %11s %9s %8s %9s %9s %8s %8s %8s %9s%n", "Size,KB", "Characters", "Tokens",
                "Load", "Preproc", "Tokenize", "Vocab", "Encode", "Export", "Total,ms"));
        sb.append("-".repeat(96)).append('\n');
        for (PerformanceResult r : results) {
            sb.append(String.format("%8d %11d %9d %8.1f %9.1f %9.1f %8.1f %8.1f %8.1f %9.1f%n",
                    r.sizeKb, r.characters, r.tokens, r.loadMs, r.preprocessMs, r.tokenizeMs,
                    r.vocabularyMs, r.encodeMs, r.exportMs, r.getTotalMs()));
        }
        return sb.toString();
    }

    public void exportCsv(List<PerformanceResult> results, Path file) throws IOException {
        if (file.getParent() != null) {
            Files.createDirectories(file.getParent());
        }
        try (BufferedWriter writer = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            writer.write("size_kb,characters,tokens,load_ms,preprocess_ms,tokenize_ms,vocabulary_ms,encode_ms,export_ms");
            writer.newLine();
            for (PerformanceResult r : results) {
                // у CSV завжди крапка, бо кома — роздільник колонок
                writer.write(String.format(Locale.US, "%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
                        r.sizeKb, r.characters, r.tokens, r.loadMs, r.preprocessMs, r.tokenizeMs,
                        r.vocabularyMs, r.encodeMs, r.exportMs));
                writer.newLine();
            }
        }
    }
}
