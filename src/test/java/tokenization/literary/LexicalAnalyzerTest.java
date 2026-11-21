package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import static org.junit.jupiter.api.Assertions.assertEquals;

class LexicalAnalyzerTest {

    private final StyleProfiler profiler = new StyleProfiler();

    private StyleProfile profile(String text) {
        return profiler.analyze(new Document(1, "test.txt", text));
    }

    @Test
    void countsWordsUniqueWordsAndLemmas() {
        // будинок / будинку / будинки — три словоформи однієї леми
        StyleProfile profile = profile("Будинок стоїть. Біля будинку сад. Будинки старі.");

        assertEquals(7, profile.getWordCount());
        assertEquals(7, profile.getUniqueWordCount());
        assertEquals(5, profile.getUniqueLemmaCount());
        assertEquals(3, profile.getSentenceCount());
    }

    @Test
    void computesTypeTokenRatio() {
        // 7 слів, 4 унікальні: кіт, і, пес, спить
        StyleProfile profile = profile("Кіт і пес. Кіт і пес спить.");
        assertEquals(7, profile.getWordCount());
        assertEquals(4.0 / 7, profile.getLexicalDiversity(), 1e-9);
    }

    @Test
    void computesAverageWordLengthAndSyllables() {
        StyleProfile profile = profile("Мама мила раму");
        assertEquals(4.0, profile.getAverageWordLength(), 1e-9);
        assertEquals(2.0, profile.getAverageSyllables(), 1e-9);
    }

    @Test
    void emptyTextGivesZeros() {
        StyleProfile profile = profile("");
        assertEquals(0, profile.getWordCount());
        assertEquals(0.0, profile.getLexicalDiversity());
    }
}
