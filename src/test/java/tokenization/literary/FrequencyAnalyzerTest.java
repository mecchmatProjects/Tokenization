package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;

class FrequencyAnalyzerTest {

    private StyleProfile profile(String text) {
        return new StyleProfiler().analyze(new Document(1, "test.txt", text));
    }

    @Test
    void findsMostFrequentWords() {
        StyleProfile profile = profile("І день, і ніч, і знову день.");
        Map.Entry<String, Integer> top = profile.topWords(1).get(0);
        assertEquals("і", top.getKey());
        assertEquals(3, top.getValue());
    }

    @Test
    void contentWordsExcludeStopWords() {
        StyleProfile profile = profile("І день, і ніч, і знову день.");
        Map.Entry<String, Integer> top = profile.topContentWords(1).get(0);
        assertEquals("день", top.getKey());
        assertEquals(2, top.getValue());
    }

    @Test
    void countsLemmas() {
        StyleProfile profile = profile("Будинок, будинку, будинки.");
        assertEquals(3, profile.getLemmaFrequencies().get("будинок"));
    }
}
