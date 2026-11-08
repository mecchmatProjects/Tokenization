package tokenization.io;

import tokenization.core.Token;
import tokenization.pipeline.Dataset;
import tokenization.pipeline.ProcessedDocument;
import tokenization.vocabulary.Vocabulary;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

/**
 * Експорт результатів обробки у CSV (README §28).
 */
public class CsvExporter {

    /** Формат: document_id,position,token,type. position — порядковий номер токена в документі. */
    public void exportTokens(List<ProcessedDocument> documents, Path file) throws IOException {
        try (BufferedWriter writer = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            writer.write("document_id,position,token,type");
            writer.newLine();
            for (ProcessedDocument document : documents) {
                List<Token> tokens = document.getNormalizedTokens();
                for (int i = 0; i < tokens.size(); i++) {
                    Token token = tokens.get(i);
                    writer.write(document.getDocument().getId() + "," + i + ","
                            + escape(token.getNormalizedText()) + "," + token.getType());
                    writer.newLine();
                }
            }
        }
    }

    /** Формат: id,token,frequency. */
    public void exportVocabulary(Vocabulary vocabulary, Path file) throws IOException {
        try (BufferedWriter writer = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            writer.write("id,token,frequency");
            writer.newLine();
            for (int id = 0; id < vocabulary.size(); id++) {
                String token = vocabulary.getToken(id);
                writer.write(id + "," + escape(token) + "," + vocabulary.getFrequency(token));
                writer.newLine();
            }
        }
    }

    /**
     * Формат: document_id,tokens, де tokens — ID через пробіл.
     *
     * @param padded true — послідовності після padding/truncation, false — повні послідовності
     */
    public void exportEncoded(List<ProcessedDocument> documents, Path file, boolean padded) throws IOException {
        try (BufferedWriter writer = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            writer.write("document_id,tokens");
            writer.newLine();
            for (ProcessedDocument document : documents) {
                StringBuilder ids = new StringBuilder();
                if (padded) {
                    for (int id : document.getPadded()) {
                        appendWithSpace(ids, id);
                    }
                } else {
                    for (int id : document.getEncoded()) {
                        appendWithSpace(ids, id);
                    }
                }
                writer.write(document.getDocument().getId() + ",\"" + ids + "\"");
                writer.newLine();
            }
        }
    }

    /** Розріджений формат document_id,token_id,value — лише ненульові значення. */
    public void exportBagOfWords(Dataset dataset, Path file) throws IOException {
        List<int[]> vectors = dataset.bagOfWords();
        try (BufferedWriter writer = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            writer.write("document_id,token_id,value");
            writer.newLine();
            for (int d = 0; d < vectors.size(); d++) {
                int documentId = dataset.getDocuments().get(d).getDocument().getId();
                int[] vector = vectors.get(d);
                for (int tokenId = 0; tokenId < vector.length; tokenId++) {
                    if (vector[tokenId] != 0) {
                        writer.write(documentId + "," + tokenId + "," + vector[tokenId]);
                        writer.newLine();
                    }
                }
            }
        }
    }

    /** Записує всі файли датасету в каталог. */
    public void exportAll(Dataset dataset, Path directory) throws IOException {
        Files.createDirectories(directory);
        exportTokens(dataset.getDocuments(), directory.resolve("tokens.csv"));
        exportVocabulary(dataset.getVocabulary(), directory.resolve("vocabulary.csv"));
        exportEncoded(dataset.getDocuments(), directory.resolve("encoded.csv"), true);
        exportBagOfWords(dataset, directory.resolve("bag_of_words.csv"));
    }

    private void appendWithSpace(StringBuilder sb, int value) {
        if (sb.length() > 0) {
            sb.append(' ');
        }
        sb.append(value);
    }

    /** Значення з комою, лапками або перенесенням рядка береться в лапки. */
    static String escape(String value) {
        if (value.contains(",") || value.contains("\"") || value.contains("\n")) {
            return "\"" + value.replace("\"", "\"\"") + "\"";
        }
        return value;
    }
}
