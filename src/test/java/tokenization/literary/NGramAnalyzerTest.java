package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

class NGramAnalyzerTest {

    private final StyleProfiler profiler = new StyleProfiler();
    private static final String TEXT = "В цей день було тепло. В цей день ми гуляли. В цей день.";

    @Test
    void countsBigramsAndTrigrams() {
        StyleProfile profile = profiler.analyze(new Document(1, "test.txt", TEXT));

        assertEquals(3, profile.getBigramFrequencies().get("в цей"));
        assertEquals(3, profile.getBigramFrequencies().get("цей день"));
        assertEquals(3, profile.getTrigramFrequencies().get("в цей день"));
        assertEquals(2, profile.getRepeatedBigrams());
        assertEquals(1, profile.getRepeatedTrigrams());
    }

    @Test
    void ngramsDoNotCrossSentences() {
        StyleProfile profile = profiler.analyze(new Document(1, "test.txt", TEXT));
        assertFalse(profile.getBigramFrequencies().containsKey("тепло в"));
    }

    @Test
    void supportsAnyN() {
        AnalyzedText text = profiler.prepare(new Document(1, "test.txt", TEXT));
        Map<String, Integer> fourGrams = NGramAnalyzer.countNGrams(text.getSentences(), 4);
        assertEquals(1, fourGrams.get("в цей день було"));
    }

    @Test
    void computesRepeatedShare() {
        // біграми: "a b" ×2, "b c" ×1 → повторів 2 з 3
        assertEquals(2.0 / 3, NGramAnalyzer.repeatedShare(Map.of("a b", 2, "b c", 1)), 1e-9);
    }
}
