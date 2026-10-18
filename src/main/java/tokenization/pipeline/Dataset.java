package tokenization.pipeline;

import tokenization.encoding.BagOfWordsEncoder;
import tokenization.vocabulary.Vocabulary;

import java.util.ArrayList;
import java.util.List;

/**
 * Підготовлений набір даних для ML: документи, словник і числове представлення.
 */
public class Dataset {

    private final List<ProcessedDocument> documents;
    private final Vocabulary vocabulary;
    private final int maxLength;

    public Dataset(List<ProcessedDocument> documents, Vocabulary vocabulary, int maxLength) {
        this.documents = documents;
        this.vocabulary = vocabulary;
        this.maxLength = maxLength;
    }

    public List<ProcessedDocument> getDocuments() {
        return documents;
    }

    public Vocabulary getVocabulary() {
        return vocabulary;
    }

    public int getMaxLength() {
        return maxLength;
    }

    /** Bag-of-Words вектор для кожного документа. */
    public List<int[]> bagOfWords() {
        BagOfWordsEncoder encoder = new BagOfWordsEncoder(vocabulary);
        List<int[]> vectors = new ArrayList<>();
        for (ProcessedDocument document : documents) {
            vectors.add(encoder.encode(document.getNormalizedTexts()));
        }
        return vectors;
    }
}
