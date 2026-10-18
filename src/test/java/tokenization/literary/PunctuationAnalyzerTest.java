package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import static org.junit.jupiter.api.Assertions.assertEquals;

class PunctuationAnalyzerTest {

    private StyleProfile profile(String text) {
        return new StyleProfiler().analyze(new Document(1, "test.txt", text));
    }

    @Test
    void countsEachMark() {
        StyleProfile profile = profile("Ну, що ж... Ходімо! Куди? Туди, де — тихо.");

        assertEquals(2, profile.getPunctuationCount(","));
        assertEquals(1, profile.getPunctuationCount("..."));
        assertEquals(1, profile.getPunctuationCount("!"));
        assertEquals(1, profile.getPunctuationCount("?"));
        assertEquals(1, profile.getPunctuationCount("—"));
        assertEquals(1, profile.getPunctuationCount("."));
    }

    @Test
    void computesPerSentenceRatios() {
        StyleProfile profile = profile("Ну, що ж... Ходімо! Куди? Туди, де — тихо.");
        assertEquals(4, profile.getSentenceCount());
        assertEquals(2.0 / 4, profile.getCommasPerSentence(), 1e-9);
        assertEquals(7.0 / 4, profile.getPunctuationPerSentence(), 1e-9);
    }

    @Test
    void unifiesEllipsisAndQuotes() {
        StyleProfile profile = profile("Так… Він сказав «ні».");
        assertEquals(1, profile.getPunctuationCount("..."));
        assertEquals(2, profile.getPunctuationCount("\""));
    }
}
