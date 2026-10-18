package tokenization.pipeline;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.encoding.IntegerEncoder;
import tokenization.normalization.Normalizer;
import tokenization.preprocessing.TextPreprocessor;
import tokenization.tokenizer.RuleBasedTokenizer;
import tokenization.tokenizer.Tokenizer;
import tokenization.vocabulary.Vocabulary;

import java.util.ArrayList;
import java.util.List;

/**
 * Повний pipeline обробки (README §12):
 * текст → попередня обробка → токенізація → нормалізація → словник → числове представлення.
 *
 * Етапи можна викликати окремо (як у консольному меню) або всі разом через process().
 */
public class TextProcessor {

    private TextPreprocessor preprocessor = new TextPreprocessor();
    private Tokenizer tokenizer = new RuleBasedTokenizer();
    private Normalizer normalizer = new Normalizer();

    private int minFrequency = 1;
    private int maxVocabularySize = 0; // 0 — без обмеження
    private int maxLength = 50;
    private boolean addStartEnd = false;

    /** Попередня обробка, токенізація та нормалізація кожного документа. */
    public List<ProcessedDocument> tokenize(List<Document> documents) {
        List<ProcessedDocument> result = new ArrayList<>();
        for (Document document : documents) {
            String text = preprocessor.process(document.getContent());
            List<Token> tokens = tokenizer.tokenize(text);
            List<Token> normalized = normalizer.normalize(tokens);
            result.add(new ProcessedDocument(document, text, tokens, normalized));
        }
        return result;
    }

    public Vocabulary buildVocabulary(List<ProcessedDocument> documents) {
        List<List<String>> texts = new ArrayList<>();
        for (ProcessedDocument document : documents) {
            texts.add(document.getNormalizedTexts());
        }
        return Vocabulary.build(texts, minFrequency, maxVocabularySize);
    }

    /** Перетворює документи на послідовності ID та вирівнює їх до maxLength. */
    public void encode(List<ProcessedDocument> documents, Vocabulary vocabulary) {
        IntegerEncoder encoder = new IntegerEncoder(vocabulary);
        encoder.setAddStartEnd(addStartEnd);

        for (ProcessedDocument document : documents) {
            List<Integer> ids = encoder.encode(document.getNormalizedTexts());
            document.setEncoded(ids);
            document.setPadded(IntegerEncoder.pad(ids, maxLength));

            // ID також зберігається в самих токенах (README §6, поле tokenId)
            List<Token> tokens = document.getNormalizedTokens();
            for (Token token : tokens) {
                token.setTokenId(vocabulary.getId(token.getNormalizedText()));
            }
        }
    }

    public Dataset process(List<Document> documents) {
        List<ProcessedDocument> processed = tokenize(documents);
        Vocabulary vocabulary = buildVocabulary(processed);
        encode(processed, vocabulary);
        return new Dataset(processed, vocabulary, maxLength);
    }

    public TextPreprocessor getPreprocessor() {
        return preprocessor;
    }

    public void setPreprocessor(TextPreprocessor preprocessor) {
        this.preprocessor = preprocessor;
    }

    public Tokenizer getTokenizer() {
        return tokenizer;
    }

    public void setTokenizer(Tokenizer tokenizer) {
        this.tokenizer = tokenizer;
    }

    public Normalizer getNormalizer() {
        return normalizer;
    }

    public void setNormalizer(Normalizer normalizer) {
        this.normalizer = normalizer;
    }

    public int getMinFrequency() {
        return minFrequency;
    }

    public void setMinFrequency(int minFrequency) {
        this.minFrequency = minFrequency;
    }

    public int getMaxVocabularySize() {
        return maxVocabularySize;
    }

    public void setMaxVocabularySize(int maxVocabularySize) {
        this.maxVocabularySize = maxVocabularySize;
    }

    public int getMaxLength() {
        return maxLength;
    }

    public void setMaxLength(int maxLength) {
        this.maxLength = maxLength;
    }

    public boolean isAddStartEnd() {
        return addStartEnd;
    }

    public void setAddStartEnd(boolean addStartEnd) {
        this.addStartEnd = addStartEnd;
    }
}
