package tokenization.experiment;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.core.TokenType;
import tokenization.encoding.IntegerEncoder;
import tokenization.normalization.Normalizer;
import tokenization.pipeline.ProcessedDocument;
import tokenization.pipeline.TextProcessor;
import tokenization.tokenizer.CharUtils;
import tokenization.tokenizer.Tokenizer;
import tokenization.tokenizer.TokenizerFactory;
import tokenization.vocabulary.Vocabulary;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Експерименти з токенізаторами та параметрами словника (README §26, §31).
 * Кожен метод повертає готову до виведення текстову таблицю.
 */
public class TokenizerExperiment {

    /** Набір тестових текстів з README §26. */
    public static final String[] EXAMPLES = {
            "Hello, world!",
            "Email: test@example.com",
            "Price: $19.99",
            "COVID-19",
            "C++",
            "machine-learning",
            "Don't stop!",
            "Visit https://example.com today",
            "Wait... What?!",
            "м'ята, будь-який"
    };

    // розділові знаки, приклеєні до слова, вважаються помилкою об'єднання ("Hello," "world!")
    private static final String ATTACHED_PUNCTUATION = ",;:!?\"()«»";

    /** Кількість токенів для тексту в кожній зі стратегій: Whitespace, Basic, Rule-based. */
    public int[] countTokens(String text) {
        List<Tokenizer> tokenizers = TokenizerFactory.createAll(true);
        int[] counts = new int[tokenizers.size()];
        for (int i = 0; i < tokenizers.size(); i++) {
            counts[i] = tokenizers.get(i).tokenize(text).size();
        }
        return counts;
    }

    /** Таблиця README §26: скільки токенів дає кожна стратегія для тестових текстів. */
    public String compareOnExamples() {
        List<Tokenizer> tokenizers = TokenizerFactory.createAll(true);
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-32s %10s %8s %11s%n", "Text", "Whitespace", "Basic", "Rule-based"));
        sb.append("-".repeat(64)).append('\n');
        for (String example : EXAMPLES) {
            int[] counts = countTokens(example);
            sb.append(String.format("%-32s %10d %8d %11d%n", example, counts[0], counts[1], counts[2]));
        }

        sb.append("\nTokens:\n");
        for (String example : EXAMPLES) {
            sb.append(example).append('\n');
            for (Tokenizer tokenizer : tokenizers) {
                List<String> texts = new ArrayList<>();
                for (Token token : tokenizer.tokenize(example)) {
                    texts.add(token.getText());
                }
                sb.append(String.format("  %-11s %s%n", tokenizer.getName() + ":", String.join(" | ", texts)));
            }
        }
        return sb.toString();
    }

    /**
     * Експеримент 1: кількість токенів, унікальних токенів, склеєних з пунктуацією токенів
     * та час токенізації для кожної стратегії на одному наборі документів.
     */
    public String compareStrategies(List<Document> documents) {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-12s %10s %10s %10s %10s%n", "Strategy", "Tokens", "Unique", "Merged", "Time, ms"));
        sb.append("-".repeat(56)).append('\n');

        for (Tokenizer tokenizer : TokenizerFactory.createAll(true)) {
            // перший прохід "розігріває" JVM, щоб час першої стратегії не був завищеним
            tokenizeAll(tokenizer, documents);

            long start = System.nanoTime();
            List<Token> tokens = tokenizeAll(tokenizer, documents);
            double ms = (System.nanoTime() - start) / 1_000_000.0;

            Set<String> unique = new HashSet<>();
            int merged = 0;
            for (Token token : tokens) {
                unique.add(token.getText().toLowerCase());
                if (hasAttachedPunctuation(token)) {
                    merged++;
                }
            }
            sb.append(String.format("%-12s %10d %10d %10d %10.2f%n",
                    tokenizer.getName(), tokens.size(), unique.size(), merged, ms));
        }
        sb.append("\nMerged = word or number with punctuation attached at the edge (\"Hello,\", \"(word\").\n");
        return sb.toString();
    }

    private List<Token> tokenizeAll(Tokenizer tokenizer, List<Document> documents) {
        List<Token> tokens = new ArrayList<>();
        for (Document document : documents) {
            tokens.addAll(tokenizer.tokenize(document.getContent()));
        }
        return tokens;
    }

    private boolean hasAttachedPunctuation(Token token) {
        String text = token.getText();
        if (text.length() < 2 || token.getType() == TokenType.PUNCTUATION) {
            return false;
        }
        boolean hasWordChar = false;
        for (int i = 0; i < text.length(); i++) {
            if (CharUtils.isWordChar(text.charAt(i))) {
                hasWordChar = true;
                break;
            }
        }
        char first = text.charAt(0);
        char last = text.charAt(text.length() - 1);
        return hasWordChar && (ATTACHED_PUNCTUATION.indexOf(first) >= 0 || ATTACHED_PUNCTUATION.indexOf(last) >= 0);
    }

    /** Експеримент 2: розмір словника з урахуванням регістру та без. */
    public String lowercaseExperiment(List<Document> documents) {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-16s %16s%n", "Mode", "Vocabulary size"));
        sb.append("-".repeat(33)).append('\n');

        sb.append(String.format("%-16s %16d%n", "case-sensitive", vocabularySize(documents, false)));
        sb.append(String.format("%-16s %16d%n", "lowercase", vocabularySize(documents, true)));
        return sb.toString();
    }

    public int vocabularySize(List<Document> documents, boolean lowercase) {
        TextProcessor processor = new TextProcessor();
        processor.getNormalizer().setLowercase(lowercase);
        return processor.buildVocabulary(processor.tokenize(documents)).size();
    }

    /** Експеримент 3: залежність розміру словника та кількості <UNK> від minFrequency. */
    public String minFrequencyExperiment(List<Document> documents, int[] values) {
        TextProcessor processor = new TextProcessor();
        List<ProcessedDocument> processed = processor.tokenize(documents);
        List<List<String>> texts = texts(processed);

        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-14s %16s %14s%n", "minFrequency", "Vocabulary size", "Unknown"));
        sb.append("-".repeat(46)).append('\n');
        for (int minFrequency : values) {
            Vocabulary vocabulary = Vocabulary.build(texts, minFrequency, 0);
            sb.append(String.format("%-14d %16d %14d%n",
                    minFrequency, vocabulary.size(), countUnknown(texts, vocabulary)));
        }
        return sb.toString();
    }

    /**
     * Експеримент 4: обмеження розміру словника.
     * Пам'ять оцінюється наближено: 2 байти на символ токена + 4 байти на його ID.
     */
    public String vocabularyLimitExperiment(List<Document> documents, int[] sizes) {
        TextProcessor processor = new TextProcessor();
        List<ProcessedDocument> processed = processor.tokenize(documents);
        List<List<String>> texts = texts(processed);

        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-10s %12s %10s %14s %14s%n",
                "Max size", "Vocabulary", "Unknown", "Memory, bytes", "Encode, ms"));
        sb.append("-".repeat(64)).append('\n');
        for (int maxSize : sizes) {
            Vocabulary vocabulary = Vocabulary.build(texts, 1, maxSize);

            long start = System.nanoTime();
            IntegerEncoder encoder = new IntegerEncoder(vocabulary);
            for (List<String> document : texts) {
                encoder.encode(document);
            }
            double ms = (System.nanoTime() - start) / 1_000_000.0;

            sb.append(String.format("%-10d %12d %10d %14d %14.2f%n", maxSize, vocabulary.size(),
                    countUnknown(texts, vocabulary), estimateMemory(vocabulary), ms));
        }
        return sb.toString();
    }

    /** Експеримент 5: скільки місця займає padding для різних maxLength. */
    public String paddingExperiment(List<Document> documents, int[] lengths) {
        TextProcessor processor = new TextProcessor();
        List<ProcessedDocument> processed = processor.tokenize(documents);
        List<List<String>> texts = texts(processed);
        Vocabulary vocabulary = Vocabulary.build(texts, 1, 0);

        IntegerEncoder encoder = new IntegerEncoder(vocabulary);
        List<List<Integer>> sequences = new ArrayList<>();
        for (List<String> document : texts) {
            sequences.add(encoder.encode(document));
        }

        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-10s %10s %10s %10s %12s %14s%n",
                "maxLength", "Cells", "Padding", "Share, %", "Truncated", "Memory, bytes"));
        sb.append("-".repeat(71)).append('\n');
        for (int maxLength : lengths) {
            int cells = sequences.size() * maxLength;
            int padding = countPadding(sequences, maxLength);
            int truncated = countTruncated(sequences, maxLength);
            double share = cells > 0 ? 100.0 * padding / cells : 0;
            sb.append(String.format("%-10d %10d %10d %10.1f %12d %14d%n",
                    maxLength, cells, padding, share, truncated, cells * 4L));
        }
        return sb.toString();
    }

    /** Кількість <PAD>, які буде додано до всіх послідовностей. */
    public static int countPadding(List<List<Integer>> sequences, int maxLength) {
        int count = 0;
        for (List<Integer> sequence : sequences) {
            if (sequence.size() < maxLength) {
                count += maxLength - sequence.size();
            }
        }
        return count;
    }

    /** Кількість токенів, які буде відкинуто через обрізання. */
    public static int countTruncated(List<List<Integer>> sequences, int maxLength) {
        int count = 0;
        for (List<Integer> sequence : sequences) {
            if (sequence.size() > maxLength) {
                count += sequence.size() - maxLength;
            }
        }
        return count;
    }

    private List<List<String>> texts(List<ProcessedDocument> processed) {
        List<List<String>> texts = new ArrayList<>();
        for (ProcessedDocument document : processed) {
            texts.add(Normalizer.texts(document.getNormalizedTokens()));
        }
        return texts;
    }

    private int countUnknown(List<List<String>> texts, Vocabulary vocabulary) {
        int count = 0;
        for (List<String> document : texts) {
            for (String token : document) {
                if (!vocabulary.contains(token)) {
                    count++;
                }
            }
        }
        return count;
    }

    private long estimateMemory(Vocabulary vocabulary) {
        long bytes = 0;
        for (String token : vocabulary.getTokens()) {
            bytes += token.length() * 2L + 4;
        }
        return bytes;
    }
}
