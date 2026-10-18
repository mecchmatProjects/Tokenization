package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;
import tokenization.normalization.StopWords;

import static org.junit.jupiter.api.Assertions.assertEquals;

class RepetitionAnalyzerTest {

    private static final String TEXT = "Людина йшла. Людина бачила ліс. Ліс був темний.";

    private StyleProfile analyze(int windowSize) {
        StyleProfiler profiler = new StyleProfiler();
        AnalyzedText text = profiler.prepare(new Document(1, "test.txt", TEXT));
        StyleProfile profile = new StyleProfile("test", "", 0);
        new RepetitionAnalyzer(windowSize, StopWords.loadDefault()).analyze(text, profile);
        return profile;
    }

    @Test
    void countsRepeatsInWindow() {
        // людина (позиції 0 і 2), ліс (4 і 5)
        assertEquals(2, analyze(20).getRepeatedInWindow());
        assertEquals(2, analyze(2).getRepeatedInWindow());
        assertEquals(1, analyze(1).getRepeatedInWindow());
    }

    @Test
    void countsRepeatsInAdjacentSentences() {
        assertEquals(2, analyze(20).getAdjacentSentenceRepeats());
    }

    @Test
    void countsRepeatedSentenceStarts() {
        StyleProfile profile = analyze(20);
        assertEquals(1, profile.getSentenceStartRepeats());
        assertEquals("людина", profile.topSentenceStarts(1).get(0).getKey());
    }

    @Test
    void computesRepetitionIndex() {
        // змістові слова: людина, йшла, людина, бачила, ліс, ліс, темний ("був" — stop word)
        assertEquals(2.0 / 7, analyze(20).getRepetitionIndex(), 1e-9);
    }
}
