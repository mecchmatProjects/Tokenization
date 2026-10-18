package tokenization.pipeline;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;
import tokenization.vocabulary.Vocabulary;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

class TextProcessorTest {

    @Test
    void processesReadmeExample() {
        // README §2
        Document document = new Document(1, "example", "Machine learning is amazing! I have 2 projects.");
        TextProcessor processor = new TextProcessor();

        Dataset dataset = processor.process(List.of(document));
        ProcessedDocument processed = dataset.getDocuments().get(0);

        assertEquals(List.of("machine", "learning", "is", "amazing", "!", "i", "have", "2", "projects", "."),
                processed.getNormalizedTexts());
        assertEquals(4 + 10, dataset.getVocabulary().size());
        assertEquals(10, processed.getEncoded().size());
        assertFalse(processed.getEncoded().contains(Vocabulary.UNK_ID));
        assertEquals(processor.getMaxLength(), processed.getPadded().length);
    }

    @Test
    void storesTokenIdsInTokens() {
        TextProcessor processor = new TextProcessor();
        Dataset dataset = processor.process(List.of(new Document(1, "a", "cat dog cat")));

        int catId = dataset.getVocabulary().getId("cat");
        assertEquals(catId, dataset.getDocuments().get(0).getNormalizedTokens().get(0).getTokenId());
    }

    @Test
    void unknownTokensAppearWhenVocabularyIsLimited() {
        TextProcessor processor = new TextProcessor();
        processor.setMaxVocabularySize(1);
        Dataset dataset = processor.process(List.of(new Document(1, "a", "cat dog cat")));

        assertEquals(List.of(4, Vocabulary.UNK_ID, 4), dataset.getDocuments().get(0).getEncoded());
    }

    @Test
    void buildsBagOfWordsForEachDocument() {
        TextProcessor processor = new TextProcessor();
        Dataset dataset = processor.process(List.of(
                new Document(1, "a", "cat dog cat"),
                new Document(2, "b", "bird")));

        List<int[]> vectors = dataset.bagOfWords();
        assertEquals(2, vectors.size());
        assertEquals(2, vectors.get(0)[dataset.getVocabulary().getId("cat")]);
        assertEquals(1, vectors.get(1)[dataset.getVocabulary().getId("bird")]);
    }
}
