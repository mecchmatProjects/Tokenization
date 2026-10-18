package tokenization.encoding;

import org.junit.jupiter.api.Test;
import tokenization.vocabulary.Vocabulary;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

class BagOfWordsEncoderTest {

    @Test
    void countsTokensAsInReadmeExample() {
        // README §23: "cat dog cat" → cat = 2, dog = 1, bird = 0
        Vocabulary vocabulary = Vocabulary.build(List.of(List.of("cat", "cat", "cat", "dog", "dog", "bird")), 1, 0);
        int[] vector = new BagOfWordsEncoder(vocabulary).encode(List.of("cat", "dog", "cat"));

        assertEquals(vocabulary.size(), vector.length);
        assertEquals(2, vector[vocabulary.getId("cat")]);
        assertEquals(1, vector[vocabulary.getId("dog")]);
        assertEquals(0, vector[vocabulary.getId("bird")]);
    }

    @Test
    void unknownTokensGoToUnkPosition() {
        Vocabulary vocabulary = Vocabulary.build(List.of(List.of("cat")), 1, 0);
        int[] vector = new BagOfWordsEncoder(vocabulary).encode(List.of("fish", "fox"));
        assertEquals(2, vector[Vocabulary.UNK_ID]);
    }
}
