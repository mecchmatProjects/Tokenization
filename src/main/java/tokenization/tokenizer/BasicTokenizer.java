package tokenization.tokenizer;

import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

/**
 * Стратегія 2: відокремлює слова від розділових знаків (README §11).
 *
 * Правила:
 * 1. Послідовність літер і цифр — один токен (WORD, або NUMBER якщо там лише цифри).
 * 2. Кожен розділовий знак — окремий токен (або група "...", "?!", якщо ввімкнено groupPunctuation).
 * 3. Кожен спеціальний символ — окремий токен SPECIAL.
 * 4. Пробільні символи пропускаються (або стають WHITESPACE, якщо ввімкнено keepWhitespace).
 */
public class BasicTokenizer implements Tokenizer {

    private boolean groupPunctuation = false;
    private boolean keepWhitespace = false;

    @Override
    public List<Token> tokenize(String text) {
        List<Token> tokens = new ArrayList<>();
        int i = 0;

        while (i < text.length()) {
            char c = text.charAt(i);
            int end;
            TokenType type;

            if (Character.isWhitespace(c)) {
                end = i + 1;
                while (end < text.length() && Character.isWhitespace(text.charAt(end))) {
                    end++;
                }
                if (keepWhitespace) {
                    tokens.add(new Token(text.substring(i, end), TokenType.WHITESPACE, i));
                }
                i = end;
                continue;
            }

            if (CharUtils.isWordChar(c)) {
                end = i + 1;
                boolean onlyDigits = Character.isDigit(c);
                while (end < text.length() && CharUtils.isWordChar(text.charAt(end))) {
                    if (!Character.isDigit(text.charAt(end))) {
                        onlyDigits = false;
                    }
                    end++;
                }
                type = onlyDigits ? TokenType.NUMBER : TokenType.WORD;
            } else if (CharUtils.isPunctuation(c)) {
                end = groupPunctuation ? CharUtils.punctuationGroupEnd(text, i) : i + 1;
                type = TokenType.PUNCTUATION;
            } else if (CharUtils.isSpecial(c)) {
                end = i + 1;
                type = TokenType.SPECIAL;
            } else {
                end = i + 1;
                type = TokenType.UNKNOWN;
            }

            tokens.add(new Token(text.substring(i, end), type, i));
            i = end;
        }
        return tokens;
    }

    public void setGroupPunctuation(boolean groupPunctuation) {
        this.groupPunctuation = groupPunctuation;
    }

    public void setKeepWhitespace(boolean keepWhitespace) {
        this.keepWhitespace = keepWhitespace;
    }

    @Override
    public String getName() {
        return "Basic";
    }
}
