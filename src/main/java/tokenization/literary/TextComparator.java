package tokenization.literary;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Порівняння стильових профілів двох текстів (README_LITCRITIC §19).
 *
 * Програма лише показує вимірювані характеристики та їх різницю
 * і не робить висновків про те, який стиль «кращий».
 */
public class TextComparator {

    private boolean lexical = true;
    private boolean sentence = true;
    private boolean punctuation = true;
    private boolean repetition = true;

    /** Значення вибраних груп характеристик. */
    public Map<String, Double> selectedFeatures(StyleProfile profile) {
        Map<String, Double> features = new LinkedHashMap<>();
        if (lexical) {
            features.putAll(profile.lexicalFeatures());
        }
        if (sentence) {
            features.putAll(profile.sentenceFeatures());
        }
        if (punctuation) {
            features.putAll(profile.punctuationFeatures());
        }
        if (repetition) {
            features.putAll(profile.repetitionFeatures());
        }
        return features;
    }

    public String compare(StyleProfile a, StyleProfile b) {
        Map<String, Double> featuresA = selectedFeatures(a);
        Map<String, Double> featuresB = selectedFeatures(b);

        StringBuilder sb = new StringBuilder();
        sb.append("A: ").append(describe(a)).append('\n');
        sb.append("B: ").append(describe(b)).append("\n\n");
        sb.append(String.format("%-26s %12s %12s %12s %10s%n", "Characteristic", "A", "B", "B - A", "Change"));
        sb.append("-".repeat(76)).append('\n');
        for (Map.Entry<String, Double> entry : featuresA.entrySet()) {
            double valueA = entry.getValue();
            double valueB = featuresB.get(entry.getKey());
            String change = valueA != 0 ? String.format("%+.1f%%", (valueB - valueA) / valueA * 100) : "—";
            sb.append(String.format("%-26s %12s %12s %12s %10s%n", entry.getKey(),
                    format(valueA), format(valueB), format(valueB - valueA), change));
        }

        if (lexical) {
            sb.append("\nTop content words:\n");
            List<Map.Entry<String, Integer>> topA = a.topContentWords(10);
            List<Map.Entry<String, Integer>> topB = b.topContentWords(10);
            for (int i = 0; i < Math.max(topA.size(), topB.size()); i++) {
                String left = i < topA.size() ? topA.get(i).getKey() + " (" + topA.get(i).getValue() + ")" : "";
                String right = i < topB.size() ? topB.get(i).getKey() + " (" + topB.get(i).getValue() + ")" : "";
                sb.append(String.format("  %-30s %s%n", left, right));
            }
        }
        return sb.toString();
    }

    private String describe(StyleProfile profile) {
        String author = profile.getAuthor().isEmpty() ? "unknown author" : profile.getAuthor();
        return profile.getTitle() + " (" + author + ")";
    }

    // цілі числа без дробової частини, решта — з трьома знаками після крапки
    private String format(double value) {
        if (value == Math.rint(value) && Math.abs(value) < 1e9) {
            return String.valueOf((long) value);
        }
        return String.format("%.3f", value);
    }

    public void setLexical(boolean lexical) {
        this.lexical = lexical;
    }

    public void setSentence(boolean sentence) {
        this.sentence = sentence;
    }

    public void setPunctuation(boolean punctuation) {
        this.punctuation = punctuation;
    }

    public void setRepetition(boolean repetition) {
        this.repetition = repetition;
    }
}
