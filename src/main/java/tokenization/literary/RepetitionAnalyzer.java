package tokenization.literary;

import tokenization.core.Token;
import tokenization.normalization.StopWords;
import tokenization.stats.Frequencies;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Повторюваність слів (README_LITCRITIC §14). Враховуються лише змістові слова (без stop words),
 * бо повтори "і", "в", "на" нічого не говорять про стиль.
 *
 * - повтори у вікні: слово зустрілося знову не далі ніж через windowSize слів;
 * - повтори в сусідніх реченнях: скільки слів речення вже були в попередньому реченні;
 * - повтори на початку речень: речення починається тим самим словом, що й попереднє (анафора).
 */
public class RepetitionAnalyzer implements TextAnalyzer {

    public static final int DEFAULT_WINDOW_SIZE = 20;

    private final int windowSize;
    private final StopWords stopWords;

    public RepetitionAnalyzer() {
        this(DEFAULT_WINDOW_SIZE, StopWords.loadDefault());
    }

    public RepetitionAnalyzer(int windowSize, StopWords stopWords) {
        this.windowSize = windowSize;
        this.stopWords = stopWords;
    }

    @Override
    public void analyze(AnalyzedText text, StyleProfile profile) {
        profile.windowSize = windowSize;
        countWindowRepeats(text.getWords(), profile);
        countSentenceRepeats(text.getSentences(), profile);
    }

    private void countWindowRepeats(List<Token> words, StyleProfile profile) {
        // остання позиція кожного слова в загальному списку слів
        Map<String, Integer> lastPosition = new HashMap<>();
        int contentWords = 0;

        for (int i = 0; i < words.size(); i++) {
            String word = words.get(i).getNormalizedText();
            if (stopWords.isStopWord(word)) {
                continue;
            }
            contentWords++;
            Integer last = lastPosition.get(word);
            if (last != null && i - last <= windowSize) {
                profile.repeatedInWindow++;
                Frequencies.add(profile.windowRepeatFrequencies, word);
            }
            lastPosition.put(word, i);
        }
        profile.repetitionIndex = contentWords > 0 ? (double) profile.repeatedInWindow / contentWords : 0;
    }

    private void countSentenceRepeats(List<Sentence> sentences, StyleProfile profile) {
        Set<String> previousWords = new HashSet<>();
        String previousFirst = null;

        for (Sentence sentence : sentences) {
            List<String> words = sentence.getWords();
            if (words.isEmpty()) {
                continue;
            }

            Set<String> currentWords = new HashSet<>();
            for (String word : words) {
                if (!stopWords.isStopWord(word)) {
                    currentWords.add(word);
                }
            }
            for (String word : currentWords) {
                if (previousWords.contains(word)) {
                    profile.adjacentSentenceRepeats++;
                }
            }

            String first = words.get(0);
            Frequencies.add(profile.sentenceStartFrequencies, first);
            if (first.equals(previousFirst)) {
                profile.sentenceStartRepeats++;
            }

            previousWords = currentWords;
            previousFirst = first;
        }
    }
}
