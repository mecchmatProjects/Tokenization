package tokenization.literary;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Текстовий звіт за стильовим профілем (README_LITCRITIC §26).
 */
public class ReportGenerator {

    private static final int TOP_COUNT = 10;
    private static final int CHART_WIDTH = 30;

    // назви розділових знаків у звіті, у порядку виведення
    private static final Map<String, String> PUNCTUATION_NAMES = new LinkedHashMap<>();

    static {
        PUNCTUATION_NAMES.put(",", "Comma");
        PUNCTUATION_NAMES.put(".", "Period");
        PUNCTUATION_NAMES.put("!", "Exclamation");
        PUNCTUATION_NAMES.put("?", "Question");
        PUNCTUATION_NAMES.put("...", "Ellipsis");
        PUNCTUATION_NAMES.put("?!", "Question-exclamation");
        PUNCTUATION_NAMES.put(":", "Colon");
        PUNCTUATION_NAMES.put(";", "Semicolon");
        PUNCTUATION_NAMES.put("—", "Dash");
        PUNCTUATION_NAMES.put("-", "Hyphen");
        PUNCTUATION_NAMES.put("\"", "Quotes");
        PUNCTUATION_NAMES.put("(", "Parenthesis");
    }

    public String generate(StyleProfile profile) {
        StringBuilder sb = new StringBuilder();
        sb.append("LITERARY TEXT ANALYSIS\n\n");
        sb.append("Title:  ").append(profile.getTitle()).append('\n');
        sb.append("Author: ").append(profile.getAuthor().isEmpty() ? "—" : profile.getAuthor()).append('\n');
        if (profile.getYear() > 0) {
            sb.append("Year:   ").append(profile.getYear()).append('\n');
        }

        section(sb, "GENERAL STATISTICS");
        line(sb, "Characters:", profile.getCharacters());
        line(sb, "Words:", profile.getWordCount());
        line(sb, "Sentences:", profile.getSentenceCount());

        section(sb, "LEXICAL STATISTICS");
        line(sb, "Unique words:", profile.getUniqueWordCount());
        line(sb, "Unique lemmas:", profile.getUniqueLemmaCount());
        line(sb, "TTR:", profile.getLexicalDiversity());
        line(sb, "Average word length:", profile.getAverageWordLength());
        line(sb, "Average syllables:", profile.getAverageSyllables());
        sb.append("\nTop words:\n").append(TextChart.barChart(profile.topWords(TOP_COUNT), CHART_WIDTH));
        sb.append("\nTop content words:\n").append(TextChart.barChart(profile.topContentWords(TOP_COUNT), CHART_WIDTH));

        section(sb, "SENTENCE STATISTICS");
        line(sb, "Average length:", profile.getAverageSentenceLength());
        line(sb, "Median length:", profile.getMedianSentenceLength());
        line(sb, "Minimum length:", profile.getMinSentenceLength());
        line(sb, "Maximum length:", profile.getMaxSentenceLength());
        line(sb, "Std deviation:", profile.getSentenceLengthStdDev());
        sb.append("\nWords per sentence:\n").append(TextChart.histogram(profile.getSentenceLengthDistribution(), 5, CHART_WIDTH));

        section(sb, "PUNCTUATION");
        int named = 0;
        for (Map.Entry<String, String> entry : PUNCTUATION_NAMES.entrySet()) {
            int count = profile.getPunctuationCount(entry.getKey());
            named += count;
            if (count > 0) {
                line(sb, entry.getValue() + ":", count);
            }
        }
        int total = 0;
        for (int count : profile.getPunctuationFrequencies().values()) {
            total += count;
        }
        if (total > named) {
            line(sb, "Other:", total - named);
        }
        line(sb, "Comma / sentence:", profile.getCommasPerSentence());
        line(sb, "Punctuation / sentence:", profile.getPunctuationPerSentence());

        section(sb, "REPETITION");
        line(sb, "Window size:", profile.getWindowSize());
        line(sb, "Repeats in window:", profile.getRepeatedInWindow());
        line(sb, "Repetition index:", profile.getRepetitionIndex());
        line(sb, "Adjacent sentences:", profile.getAdjacentSentenceRepeats());
        line(sb, "Sentence starts:", profile.getSentenceStartRepeats());
        line(sb, "Repeated bigrams:", profile.getRepeatedBigrams());
        line(sb, "Repeated trigrams:", profile.getRepeatedTrigrams());
        line(sb, "Phrase repetition:", profile.getPhraseRepetition());
        // для слів значення — кількість повторів, тому показуємо всі; для фраз — лише ті, що трапились двічі й більше
        appendTop(sb, "Most repeated words (repeats in window):", profile.topWindowRepeats(TOP_COUNT), 1);
        appendTop(sb, "Most frequent bigrams:", profile.topBigrams(TOP_COUNT), 2);
        appendTop(sb, "Most frequent trigrams:", profile.topTrigrams(TOP_COUNT), 2);
        return sb.toString();
    }

    public void save(String report, Path file) throws IOException {
        if (file.getParent() != null) {
            Files.createDirectories(file.getParent());
        }
        Files.writeString(file, report, StandardCharsets.UTF_8);
    }

    private void section(StringBuilder sb, String title) {
        sb.append('\n').append(title).append('\n').append("-".repeat(title.length())).append('\n');
    }

    private void line(StringBuilder sb, String label, int value) {
        sb.append(String.format("%-24s %10d%n", label, value));
    }

    private void line(StringBuilder sb, String label, double value) {
        sb.append(String.format("%-24s %10.3f%n", label, value));
    }

    /** Показує записи зі значенням не менше minValue. */
    private void appendTop(StringBuilder sb, String title, List<Map.Entry<String, Integer>> entries, int minValue) {
        sb.append('\n').append(title).append('\n');
        boolean any = false;
        for (Map.Entry<String, Integer> entry : entries) {
            if (entry.getValue() >= minValue) {
                sb.append(String.format("  %-28s %5d%n", entry.getKey(), entry.getValue()));
                any = true;
            }
        }
        if (!any) {
            sb.append("  (none)\n");
        }
    }
}
