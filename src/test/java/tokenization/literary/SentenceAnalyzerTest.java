package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import java.util.List;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;

class SentenceAnalyzerTest {

    private StyleProfile profile(String text) {
        return new StyleProfiler().analyze(new Document(1, "test.txt", text));
    }

    @Test
    void computesSentenceLengthStatistics() {
        // довжини: 1, 2, 3, 4
        StyleProfile profile = profile("Раз. Два три. Чотири п'ять шість. Сім вісім дев'ять десять.");

        assertEquals(4, profile.getSentenceCount());
        assertEquals(2.5, profile.getAverageSentenceLength(), 1e-9);
        assertEquals(2.5, profile.getMedianSentenceLength(), 1e-9);
        assertEquals(1, profile.getMinSentenceLength());
        assertEquals(4, profile.getMaxSentenceLength());
        assertEquals(Math.sqrt(1.25), profile.getSentenceLengthStdDev(), 1e-9);
        assertEquals(Map.of(1, 1, 2, 1, 3, 1, 4, 1), profile.getSentenceLengthDistribution());
    }

    @Test
    void medianOfOddCount() {
        assertEquals(3.0, SentenceAnalyzer.median(List.of(1, 3, 10)), 1e-9);
    }

    @Test
    void ignoresSentencesWithoutWords() {
        StyleProfile profile = profile("Перший рядок.\n\n***\n\nДругий рядок тут.");
        assertEquals(2, profile.getMinSentenceLength());
        assertEquals(3, profile.getMaxSentenceLength());
    }
}
