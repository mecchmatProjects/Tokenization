package tokenization.stats;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;
import tokenization.core.TokenType;
import tokenization.pipeline.Dataset;
import tokenization.pipeline.TextProcessor;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class StatisticsAnalyzerTest {

    private Dataset process(TextProcessor processor, String... texts) {
        List<Document> documents = new ArrayList<>();
        for (int i = 0; i < texts.length; i++) {
            documents.add(new Document(i + 1, "doc" + i, texts[i]));
        }
        return processor.process(documents);
    }

    @Test
    void countsBasicStatistics() {
        Dataset dataset = process(new TextProcessor(), "cat dog cat.", "Bird 2");
        CorpusStatistics stats = new StatisticsAnalyzer().analyze(dataset.getDocuments(), dataset.getVocabulary());

        assertEquals(2, stats.getDocuments());
        assertEquals(12 + 6, stats.getCharacters());
        assertEquals(6, stats.getTokens());
        assertEquals(5, stats.getUniqueTokens()); // cat, dog, ., bird, 2
        assertEquals((3 + 3 + 3 + 1 + 4 + 1) / 6.0, stats.getAverageTokenLength(), 1e-9);
        assertEquals(4 + 5, stats.getVocabularySize());
        assertEquals(0, stats.getUnknownTokens());
    }

    @Test
    void countsTokenAndTypeFrequencies() {
        Dataset dataset = process(new TextProcessor(), "cat dog cat. 2");
        CorpusStatistics stats = new StatisticsAnalyzer().analyze(dataset.getDocuments(), dataset.getVocabulary());

        assertEquals(3, stats.getTypeFrequencies().get(TokenType.WORD));
        assertEquals(1, stats.getTypeFrequencies().get(TokenType.NUMBER));
        assertEquals(1, stats.getTypeFrequencies().get(TokenType.PUNCTUATION));

        Map.Entry<String, Integer> top = stats.topTokens(1).get(0);
        assertEquals("cat", top.getKey());
        assertEquals(2, top.getValue());
    }

    @Test
    void countsUnknownTokens() {
        TextProcessor processor = new TextProcessor();
        processor.setMinFrequency(2);
        Dataset dataset = process(processor, "cat dog cat fish");
        CorpusStatistics stats = new StatisticsAnalyzer().analyze(dataset.getDocuments(), dataset.getVocabulary());

        assertEquals(2, stats.getUnknownTokens());
    }

    @Test
    void worksWithoutVocabulary() {
        Dataset dataset = process(new TextProcessor(), "a b");
        CorpusStatistics stats = new StatisticsAnalyzer().analyze(dataset.getDocuments(), null);

        assertEquals(0, stats.getVocabularySize());
        assertTrue(stats.format(5).contains("Tokens:"));
    }
}
