package tokenization.tokenizer;

import tokenization.core.Token;

import java.util.ArrayList;
import java.util.List;

/**
 * Стратегія 1: розбиття тексту лише за пробілами (README §11).
 *
 * "Hello, world!" → "Hello," "world!"
 * Розділові знаки залишаються частиною токенів.
 */
public class WhitespaceTokenizer implements Tokenizer {

    @Override
    public List<Token> tokenize(String text) {
        List<Token> tokens = new ArrayList<>();
        int start = -1; // початок поточного токена, -1 якщо ми між токенами

        for (int i = 0; i < text.length(); i++) {
            if (Character.isWhitespace(text.charAt(i))) {
                if (start >= 0) {
                    addToken(tokens, text, start, i);
                    start = -1;
                }
            } else if (start < 0) {
                start = i;
            }
        }
        if (start >= 0) {
            addToken(tokens, text, start, text.length());
        }
        return tokens;
    }

    private void addToken(List<Token> tokens, String text, int start, int end) {
        String value = text.substring(start, end);
        tokens.add(new Token(value, CharUtils.classify(value), start));
    }

    @Override
    public String getName() {
        return "Whitespace";
    }
}
