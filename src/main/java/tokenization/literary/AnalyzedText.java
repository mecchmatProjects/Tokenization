package tokenization.literary;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

/**
 * Текст, підготовлений до аналізу: токени з лемами та речення.
 */
public class AnalyzedText {

    private final Document document;
    private final String text;
    private final List<Token> tokens;
    private final List<Sentence> sentences;
    private final List<Token> words = new ArrayList<>();

    public AnalyzedText(Document document, String text, List<Token> tokens, List<Sentence> sentences) {
        this.document = document;
        this.text = text;
        this.tokens = tokens;
        this.sentences = sentences;
        for (Token token : tokens) {
            if (token.getType() == TokenType.WORD) {
                words.add(token);
            }
        }
    }

    public Document getDocument() {
        return document;
    }

    /** Текст після попередньої обробки. */
    public String getText() {
        return text;
    }

    public List<Token> getTokens() {
        return tokens;
    }

    public List<Sentence> getSentences() {
        return sentences;
    }

    /** Лише токени типу WORD. */
    public List<Token> getWords() {
        return words;
    }
}
