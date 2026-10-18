package tokenization.pipeline;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.normalization.Normalizer;

import java.util.ArrayList;
import java.util.List;

/**
 * Документ разом з результатами всіх етапів обробки.
 */
public class ProcessedDocument {

    private final Document document;
    private final String preprocessedText;
    // токени одразу після токенізатора
    private final List<Token> tokens;
    // токени після нормалізації (частина могла бути видалена)
    private final List<Token> normalizedTokens;

    private List<Integer> encoded = new ArrayList<>();
    private int[] padded = new int[0];

    public ProcessedDocument(Document document, String preprocessedText,
                             List<Token> tokens, List<Token> normalizedTokens) {
        this.document = document;
        this.preprocessedText = preprocessedText;
        this.tokens = tokens;
        this.normalizedTokens = normalizedTokens;
    }

    public Document getDocument() {
        return document;
    }

    public String getPreprocessedText() {
        return preprocessedText;
    }

    public List<Token> getTokens() {
        return tokens;
    }

    public List<Token> getNormalizedTokens() {
        return normalizedTokens;
    }

    public List<String> getNormalizedTexts() {
        return Normalizer.texts(normalizedTokens);
    }

    public List<Integer> getEncoded() {
        return encoded;
    }

    public void setEncoded(List<Integer> encoded) {
        this.encoded = encoded;
    }

    public int[] getPadded() {
        return padded;
    }

    public void setPadded(int[] padded) {
        this.padded = padded;
    }
}
