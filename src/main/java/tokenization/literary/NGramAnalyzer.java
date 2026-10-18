package tokenization.literary;

import tokenization.stats.Frequencies;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Пошук повторюваних фраз — n-грам (README_LITCRITIC §15-16).
 *
 * Використовується hash-based підхід: усі n-грами кожного речення додаються в HashMap
 * "фраза → частота". Для фіксованого n це O(N), де N — кількість слів.
 * n-грами не переходять через межу речення.
 */
public class NGramAnalyzer implements TextAnalyzer {

    @Override
    public void analyze(AnalyzedText text, StyleProfile profile) {
        profile.bigramFrequencies.putAll(countNGrams(text.getSentences(), 2));
        profile.trigramFrequencies.putAll(countNGrams(text.getSentences(), 3));
        profile.repeatedBigrams = countRepeated(profile.bigramFrequencies);
        profile.repeatedTrigrams = countRepeated(profile.trigramFrequencies);
        profile.phraseRepetition = repeatedShare(profile.bigramFrequencies);
    }

    /** Частоти всіх n-грам тексту для довільного n. */
    public static Map<String, Integer> countNGrams(List<Sentence> sentences, int n) {
        Map<String, Integer> frequencies = new HashMap<>();
        for (Sentence sentence : sentences) {
            List<String> words = sentence.getWords();
            for (int i = 0; i + n <= words.size(); i++) {
                String ngram = String.join(" ", words.subList(i, i + n));
                Frequencies.add(frequencies, ngram);
            }
        }
        return frequencies;
    }

    /** Кількість різних n-грам, що зустрічаються щонайменше двічі. */
    static int countRepeated(Map<String, Integer> frequencies) {
        int count = 0;
        for (int frequency : frequencies.values()) {
            if (frequency >= 2) {
                count++;
            }
        }
        return count;
    }

    /** Частка входжень n-грам, які є повторами (n-грама зустрічається більше одного разу). */
    static double repeatedShare(Map<String, Integer> frequencies) {
        int total = 0;
        int repeated = 0;
        for (int frequency : frequencies.values()) {
            total += frequency;
            if (frequency >= 2) {
                repeated += frequency;
            }
        }
        return total > 0 ? (double) repeated / total : 0;
    }
}
