package tokenization.normalization;

import org.junit.jupiter.api.Test;

import java.util.Set;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class StopWordsTest {

    @Test
    void loadsEnglishAndUkrainianLists() {
        StopWords stopWords = StopWords.loadDefault();
        assertTrue(stopWords.isStopWord("the"));
        assertTrue(stopWords.isStopWord("The"));
        assertTrue(stopWords.isStopWord("і"));
        assertFalse(stopWords.isStopWord("machine"));
    }

    @Test
    void customListIsCaseInsensitive() {
        StopWords stopWords = new StopWords(Set.of("Foo"));
        assertTrue(stopWords.isStopWord("foo"));
        assertTrue(stopWords.isStopWord("FOO"));
    }
}
