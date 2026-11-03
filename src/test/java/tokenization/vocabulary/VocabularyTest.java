package tokenization.vocabulary;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class VocabularyTest {

    private final List<List<String>> documents = List.of(
            List.of("machine", "learning", "is", "fun"),
            List.of("machine", "learning", "machine", "data"),
            List.of("data", "model", "machine")
    );

    @Test
    void reservesSpecialTokens() {
        Vocabulary vocabulary = new Vocabulary();
        assertEquals(4, vocabulary.size());
        assertEquals(Vocabulary.PAD_ID, vocabulary.getId("<PAD>"));
        assertEquals(Vocabulary.UNK_ID, vocabulary.getId("<UNK>"));
        assertEquals(Vocabulary.START_ID, vocabulary.getId("<START>"));
        assertEquals(Vocabulary.END_ID, vocabulary.getId("<END>"));
    }

    @Test
    void ordersTokensByFrequencyThenAlphabetically() {
        Vocabulary vocabulary = Vocabulary.build(documents, 1, 0);
        // machine ×4, data ×2, learning ×2, fun ×1, is ×1, model ×1
        assertEquals(List.of("<PAD>", "<UNK>", "<START>", "<END>",
                "machine", "data", "learning", "fun", "is", "model"), vocabulary.getTokens());
        assertEquals(4, vocabulary.getFrequency("machine"));
    }

    @Test
    void mapsInBothDirections() {
        Vocabulary vocabulary = Vocabulary.build(documents, 1, 0);
        int id = vocabulary.getId("learning");
        assertEquals("learning", vocabulary.getToken(id));
    }

    @Test
    void unknownTokenGetsUnkId() {
        Vocabulary vocabulary = Vocabulary.build(documents, 1, 0);
        assertEquals(Vocabulary.UNK_ID, vocabulary.getId("neural"));
        assertEquals("<UNK>", vocabulary.getToken(1000));
    }

    @Test
    void minFrequencyFiltersRareTokens() {
        Vocabulary vocabulary = Vocabulary.build(documents, 2, 0);
        assertTrue(vocabulary.contains("data"));
        assertFalse(vocabulary.contains("fun"));
        assertEquals(4 + 3, vocabulary.size());
    }

    @Test
    void maxSizeKeepsMostFrequentTokens() {
        Vocabulary vocabulary = Vocabulary.build(documents, 1, 2);
        assertEquals(4 + 2, vocabulary.size());
        assertTrue(vocabulary.contains("machine"));
        assertTrue(vocabulary.contains("data"));
        assertEquals(Vocabulary.UNK_ID, vocabulary.getId("model"));
    }

    @Test
    void emptyDatasetGivesOnlySpecialTokens() {
        assertEquals(4, Vocabulary.build(List.of(), 1, 0).size());
    }
}
