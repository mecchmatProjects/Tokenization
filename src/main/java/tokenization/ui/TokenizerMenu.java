package tokenization.ui;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.io.CsvExporter;
import tokenization.io.TextLoader;
import tokenization.normalization.Normalizer;
import tokenization.pipeline.Dataset;
import tokenization.pipeline.ProcessedDocument;
import tokenization.pipeline.TextProcessor;
import tokenization.preprocessing.TextPreprocessor;
import tokenization.stats.CorpusStatistics;
import tokenization.stats.StatisticsAnalyzer;
import tokenization.tokenizer.TokenizerFactory;
import tokenization.vocabulary.Vocabulary;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Консольне меню токенізатора (README §27).
 */
public class TokenizerMenu {

    // скільки токенів/ID показувати на екрані для кожного документа
    private static final int PREVIEW_SIZE = 20;

    private final ConsoleInput input;
    private final TextProcessor processor = new TextProcessor();

    private int strategy = TokenizerFactory.RULE_BASED;
    private boolean groupPunctuation = true;

    private List<Document> documents = new ArrayList<>();
    private List<ProcessedDocument> processed = new ArrayList<>();
    private Vocabulary vocabulary;
    private boolean encoded = false;

    public TokenizerMenu(ConsoleInput input) {
        this.input = input;
    }

    public void run() {
        while (true) {
            System.out.println();
            System.out.println("=== Text Tokenizer ===");
            System.out.println("1. Load document");
            System.out.println("2. Configure tokenizer");
            System.out.println("3. Tokenize");
            System.out.println("4. Build vocabulary");
            System.out.println("5. Encode dataset");
            System.out.println("6. Show statistics");
            System.out.println("7. Export dataset");
            System.out.println("8. Compare strategies");
            System.out.println("0. Back");

            int choice = input.readChoice();
            switch (choice) {
                case 1 -> loadDocuments();
                case 2 -> configure();
                case 3 -> tokenize();
                case 4 -> buildVocabulary();
                case 5 -> encode();
                case 6 -> showStatistics();
                case 7 -> export();
                case 8 -> compareStrategies();
                case 0 -> {
                    return;
                }
                default -> System.out.println("Unknown option.");
            }
        }
    }

    private void loadDocuments() {
        String path = input.readLine("File or directory", "dataset");
        String encoding = input.readLine("Encoding", "UTF-8");
        try {
            TextLoader loader = new TextLoader(Charset.forName(encoding));
            documents = loader.load(Path.of(path));
            processed = new ArrayList<>();
            vocabulary = null;
            encoded = false;
            System.out.println("Loaded documents: " + documents.size());
            for (Document document : documents) {
                System.out.println("  " + document.getId() + ". " + document.getName()
                        + " (" + document.getContent().length() + " chars)");
            }
        } catch (IOException | IllegalArgumentException e) {
            System.out.println("Cannot load: " + e.getMessage());
        }
    }

    private void configure() {
        TextPreprocessor preprocessor = processor.getPreprocessor();
        Normalizer normalizer = processor.getNormalizer();

        while (true) {
            System.out.println();
            System.out.println("--- Tokenizer settings ---");
            System.out.println(" 1. Strategy:               " + processor.getTokenizer().getName());
            System.out.println(" 2. Group punctuation:      " + groupPunctuation + "  (\"...\", \"?!\" as one token)");
            System.out.println(" 3. Join lines:             " + preprocessor.isJoinLines());
            System.out.println(" 4. Lowercase:              " + normalizer.isLowercase());
            System.out.println(" 5. Remove punctuation:     " + normalizer.isRemovePunctuation());
            System.out.println(" 6. Remove stop words:      " + normalizer.isRemoveStopWords());
            System.out.println(" 7. Replace numbers:        " + normalizer.isReplaceNumbers());
            System.out.println(" 8. Replace URLs / emails:  " + normalizer.isReplaceUrls());
            System.out.println(" 9. Lemmatization:          " + normalizer.isLemmatize());
            System.out.println("10. Min frequency:          " + processor.getMinFrequency());
            System.out.println("11. Max vocabulary size:    " + processor.getMaxVocabularySize() + "  (0 = no limit)");
            System.out.println("12. Max sequence length:    " + processor.getMaxLength());
            System.out.println("13. Add <START>/<END>:      " + processor.isAddStartEnd());
            System.out.println(" 0. Back");

            int choice = input.readChoice();
            switch (choice) {
                case 1 -> {
                    System.out.println("1. Whitespace  2. Basic  3. Rule-based");
                    int value = input.readInt("Strategy", strategy);
                    if (value >= 1 && value <= 3) {
                        strategy = value;
                    }
                }
                case 2 -> groupPunctuation = !groupPunctuation;
                case 3 -> preprocessor.setJoinLines(!preprocessor.isJoinLines());
                case 4 -> normalizer.setLowercase(!normalizer.isLowercase());
                case 5 -> normalizer.setRemovePunctuation(!normalizer.isRemovePunctuation());
                case 6 -> normalizer.setRemoveStopWords(!normalizer.isRemoveStopWords());
                case 7 -> normalizer.setReplaceNumbers(!normalizer.isReplaceNumbers());
                case 8 -> {
                    boolean value = !normalizer.isReplaceUrls();
                    normalizer.setReplaceUrls(value);
                    normalizer.setReplaceEmails(value);
                }
                case 9 -> normalizer.setLemmatize(!normalizer.isLemmatize());
                case 10 -> processor.setMinFrequency(input.readInt("Min frequency", processor.getMinFrequency()));
                case 11 -> processor.setMaxVocabularySize(
                        input.readInt("Max vocabulary size", processor.getMaxVocabularySize()));
                case 12 -> processor.setMaxLength(input.readInt("Max sequence length", processor.getMaxLength()));
                case 13 -> processor.setAddStartEnd(!processor.isAddStartEnd());
                case 0 -> {
                    return;
                }
                default -> System.out.println("Unknown option.");
            }
            processor.setTokenizer(TokenizerFactory.create(strategy, groupPunctuation));
        }
    }

    private void tokenize() {
        if (documents.isEmpty()) {
            System.out.println("Load documents first.");
            return;
        }
        processed = processor.tokenize(documents);
        vocabulary = null;
        encoded = false;

        for (ProcessedDocument document : processed) {
            List<Token> tokens = document.getNormalizedTokens();
            System.out.println(document.getDocument().getName() + ": " + tokens.size() + " tokens");
            StringBuilder preview = new StringBuilder("  ");
            for (int i = 0; i < Math.min(PREVIEW_SIZE, tokens.size()); i++) {
                preview.append(tokens.get(i).getNormalizedText()).append(" | ");
            }
            if (tokens.size() > PREVIEW_SIZE) {
                preview.append("...");
            }
            System.out.println(preview);
        }
    }

    private void buildVocabulary() {
        if (processed.isEmpty()) {
            System.out.println("Tokenize documents first.");
            return;
        }
        vocabulary = processor.buildVocabulary(processed);
        encoded = false;

        System.out.println("Vocabulary size: " + vocabulary.size());
        for (int id = 0; id < Math.min(PREVIEW_SIZE, vocabulary.size()); id++) {
            String token = vocabulary.getToken(id);
            System.out.printf("  %4d  %-20s %d%n", id, token, vocabulary.getFrequency(token));
        }
        if (vocabulary.size() > PREVIEW_SIZE) {
            System.out.println("  ...");
        }
    }

    private void encode() {
        if (vocabulary == null) {
            System.out.println("Build vocabulary first.");
            return;
        }
        processor.encode(processed, vocabulary);
        encoded = true;

        for (ProcessedDocument document : processed) {
            List<Integer> ids = document.getEncoded();
            List<Integer> preview = ids.subList(0, Math.min(PREVIEW_SIZE, ids.size()));
            System.out.println(document.getDocument().getName() + ": " + ids.size() + " ids " + preview
                    + (ids.size() > PREVIEW_SIZE ? " ..." : ""));
        }
        if (!processed.isEmpty()) {
            int[] padded = processed.get(0).getPadded();
            System.out.println("Padded (maxLength = " + padded.length + "), first document:");
            System.out.println("  " + Arrays.toString(padded));
        }
    }

    private void showStatistics() {
        if (processed.isEmpty()) {
            System.out.println("Tokenize documents first.");
            return;
        }
        CorpusStatistics stats = new StatisticsAnalyzer().analyze(processed, vocabulary);
        System.out.println(stats.format(10));
    }

    private void export() {
        if (!encoded) {
            System.out.println("Encode dataset first.");
            return;
        }
        String directory = input.readLine("Output directory", "output");
        try {
            Dataset dataset = new Dataset(processed, vocabulary, processor.getMaxLength());
            new CsvExporter().exportAll(dataset, Path.of(directory));
            System.out.println("Saved tokens.csv, vocabulary.csv, encoded.csv, bag_of_words.csv to " + directory);
        } catch (IOException e) {
            System.out.println("Cannot export: " + e.getMessage());
        }
    }

    private void compareStrategies() {
        // TODO: таблиця порівняння стратегій та експерименти (README §26, §31)
        System.out.println("Not implemented yet.");
    }
}
