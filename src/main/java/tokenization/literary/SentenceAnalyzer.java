package tokenization.literary;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Аналіз довжин речень у словах (README_LITCRITIC §11-12):
 * середнє, медіана, мінімум, максимум, стандартне відхилення та розподіл довжин.
 * Речення без жодного слова (наприклад, "***") не враховуються.
 */
public class SentenceAnalyzer implements TextAnalyzer {

    @Override
    public void analyze(AnalyzedText text, StyleProfile profile) {
        List<Integer> lengths = new ArrayList<>();
        for (Sentence sentence : text.getSentences()) {
            if (sentence.getWordCount() > 0) {
                lengths.add(sentence.getWordCount());
            }
        }
        if (lengths.isEmpty()) {
            return;
        }
        Collections.sort(lengths);

        double sum = 0;
        for (int length : lengths) {
            sum += length;
            profile.sentenceLengthDistribution.put(length,
                    profile.sentenceLengthDistribution.getOrDefault(length, 0) + 1);
        }
        double average = sum / lengths.size();

        double squares = 0;
        for (int length : lengths) {
            squares += (length - average) * (length - average);
        }

        profile.averageSentenceLength = average;
        profile.medianSentenceLength = median(lengths);
        profile.minSentenceLength = lengths.get(0);
        profile.maxSentenceLength = lengths.get(lengths.size() - 1);
        profile.sentenceLengthStdDev = Math.sqrt(squares / lengths.size());
    }

    /** Медіана відсортованого списку. */
    static double median(List<Integer> sorted) {
        int middle = sorted.size() / 2;
        if (sorted.size() % 2 == 1) {
            return sorted.get(middle);
        }
        return (sorted.get(middle - 1) + sorted.get(middle)) / 2.0;
    }
}
