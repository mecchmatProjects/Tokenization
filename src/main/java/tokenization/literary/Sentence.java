package tokenization.literary;

import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

/**
 * Речення тексту (README_LITCRITIC §11).
 */
public class Sentence {

    private final int number;
    private final String text;
    private final List<Token> tokens;

    public Sentence(int number, String text, List<Token> tokens) {
        this.number = number;
        this.text = text;
        this.tokens = tokens;
    }

    public int getNumber() {
        return number;
    }

    public String getText() {
        return text;
    }

    public List<Token> getTokens() {
        return tokens;
    }

    public int getTokenCount() {
        return tokens.size();
    }

    public int getWordCount() {
        return countType(TokenType.WORD);
    }

    public int getPunctuationCount() {
        return countType(TokenType.PUNCTUATION);
    }

    /** Нормалізовані слова речення (без пунктуації та чисел). */
    public List<String> getWords() {
        List<String> words = new ArrayList<>();
        for (Token token : tokens) {
            if (token.getType() == TokenType.WORD) {
                words.add(token.getNormalizedText());
            }
        }
        return words;
    }

    private int countType(TokenType type) {
        int count = 0;
        for (Token token : tokens) {
            if (token.getType() == type) {
                count++;
            }
        }
        return count;
    }
}
