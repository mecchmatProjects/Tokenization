package tokenization.encoding;

import org.junit.jupiter.api.Test;
import tokenization.vocabulary.Vocabulary;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;

class IntegerEncoderTest {

    private final Vocabulary vocabulary = Vocabulary.build(
            List.of(List.of("machine", "learning", "is", "useful")), 1, 0);

    @Test
    void encodesTokensToIds() {
        IntegerEncoder encoder = new IntegerEncoder(vocabulary);
        List<Integer> ids = encoder.encode(List.of("machine", "learning", "unknown"));

        assertEquals(3, ids.size());
        assertEquals(vocabulary.getId("machine"), ids.get(0));
        assertEquals(Vocabulary.UNK_ID, ids.get(2));
    }

    @Test
    void addsStartAndEndTokens() {
        IntegerEncoder encoder = new IntegerEncoder(vocabulary);
        encoder.setAddStartEnd(true);
        List<String> decoded = encoder.decode(encoder.encode(List.of("machine", "learning", "is", "useful")));

        // README §17: <START> machine learning is useful <END>
        assertEquals(List.of("<START>", "machine", "learning", "is", "useful", "<END>"), decoded);
    }

    @Test
    void padsAndTruncatesAsInReadmeExample() {
        // README §21, maxLength = 5
        assertArrayEquals(new int[]{2, 5, 7, 0, 0}, IntegerEncoder.pad(List.of(2, 5, 7), 5));
        assertArrayEquals(new int[]{3, 8, 1, 9, 4}, IntegerEncoder.pad(List.of(3, 8, 1, 9, 4), 5));
        assertArrayEquals(new int[]{6, 2, 0, 0, 0}, IntegerEncoder.pad(List.of(6, 2), 5));
        assertArrayEquals(new int[]{1, 2, 3}, IntegerEncoder.pad(List.of(1, 2, 3, 4, 5), 3));
    }
}
