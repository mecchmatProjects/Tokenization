package tokenization.literary;

import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

/**
 * Визначення меж речень за токенами (README_LITCRITIC §11).
 *
 * Речення закінчується:
 * - розділовим знаком із символів ".!?…" ("." "!" "?!" "..."); закриваючі лапки й дужки
 *   одразу після нього належать цьому ж реченню («Привіт!»);
 * - порожнім рядком між абзацами (заголовки без крапки).
 * Скорочення (т.д., Dr.) Rule-based токенізатор повертає разом із крапкою, тому вони речення не розривають.
 */
public class SentenceSplitter {

    private static final String END_CHARS = ".!?…";
    private static final String CLOSING_CHARS = "»\"”)]'";

    /**
     * @param tokens токени тексту
     * @param text   текст, з якого отримано токени (потрібен для тексту речень і пошуку абзаців)
     */
    public List<Sentence> split(List<Token> tokens, String text) {
        List<Sentence> sentences = new ArrayList<>();
        List<Token> current = new ArrayList<>();

        for (int i = 0; i < tokens.size(); i++) {
            Token token = tokens.get(i);
            current.add(token);

            boolean end = false;
            if (isSentenceEnd(token)) {
                end = true;
                while (i + 1 < tokens.size() && isClosing(tokens.get(i + 1))) {
                    i++;
                    current.add(tokens.get(i));
                }
            } else if (i + 1 < tokens.size() && hasParagraphBreak(text, token, tokens.get(i + 1))) {
                end = true;
            }

            if (end || i == tokens.size() - 1) {
                sentences.add(createSentence(sentences.size() + 1, current, text));
                current = new ArrayList<>();
            }
        }
        return sentences;
    }

    private Sentence createSentence(int number, List<Token> tokens, String text) {
        for (Token token : tokens) {
            token.setSentenceNumber(number);
        }
        Token first = tokens.get(0);
        Token last = tokens.get(tokens.size() - 1);
        String sentenceText = text.substring(first.getPosition(), last.getPosition() + last.getText().length());
        return new Sentence(number, sentenceText, tokens);
    }

    private boolean isSentenceEnd(Token token) {
        if (token.getType() != TokenType.PUNCTUATION) {
            return false;
        }
        String value = token.getText();
        for (int i = 0; i < value.length(); i++) {
            if (END_CHARS.indexOf(value.charAt(i)) < 0) {
                return false;
            }
        }
        return true;
    }

    private boolean isClosing(Token token) {
        return token.getType() == TokenType.PUNCTUATION && token.getText().length() == 1
                && CLOSING_CHARS.indexOf(token.getText().charAt(0)) >= 0;
    }

    private boolean hasParagraphBreak(String text, Token token, Token next) {
        int end = token.getPosition() + token.getText().length();
        return text.substring(end, next.getPosition()).contains("\n\n");
    }
}
