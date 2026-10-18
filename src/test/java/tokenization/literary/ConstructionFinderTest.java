package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class ConstructionFinderTest {

    private final AnalyzedText text = new StyleProfiler().prepare(new Document(1, "test.txt",
            "Якщо підеш, то побачиш. Не тільки він, а й вона прийшла. То було давно, якщо не помиляюсь."));
    private final ConstructionFinder finder = new ConstructionFinder();

    @Test
    void findsConstructionInOrder() {
        List<Sentence> found = finder.find(text, "якщо ... то ...");
        assertEquals(1, found.size());
        assertTrue(found.get(0).getText().startsWith("Якщо підеш"));
    }

    @Test
    void findsMultiWordParts() {
        assertEquals(1, finder.find(text, "не тільки ... а й ...").size());
    }

    @Test
    void returnsEmptyWhenNothingMatches() {
        assertTrue(finder.find(text, "так ... як ...").isEmpty());
        assertTrue(finder.find(text, "...").isEmpty());
    }

    @Test
    void parsesPattern() {
        assertEquals(List.of(List.of("не", "тільки"), List.of("а", "й")),
                ConstructionFinder.parsePattern("Не тільки ... а й ..."));
    }
}
