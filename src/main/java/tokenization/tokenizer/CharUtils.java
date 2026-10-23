package tokenization.tokenizer;

import tokenization.core.TokenType;

/**
 * Допоміжні методи для визначення класу символу. Спільні для всіх токенізаторів.
 */
public class CharUtils {

    // README §10. Частина з них за Unicode належить до пунктуації, тому перевіряємо їх першими
    private static final String SPECIAL_CHARS = "@#$%&*+=/\\_|<>^~`";

    // символи, з яких можуть складатися багатосимвольні знаки: "...", "?!", "!!"
    private static final String SENTENCE_END_CHARS = ".!?";

    private CharUtils() {
    }

    /** Символ, який може бути частиною слова: літера, цифра або комбінований знак (наголос). */
    public static boolean isWordChar(char c) {
        return Character.isLetterOrDigit(c) || Character.getType(c) == Character.NON_SPACING_MARK;
    }

    public static boolean isSpecial(char c) {
        if (SPECIAL_CHARS.indexOf(c) >= 0) {
            return true;
        }
        int type = Character.getType(c);
        return type == Character.MATH_SYMBOL
                || type == Character.CURRENCY_SYMBOL
                || type == Character.MODIFIER_SYMBOL
                || type == Character.OTHER_SYMBOL;
    }

    public static boolean isPunctuation(char c) {
        if (SPECIAL_CHARS.indexOf(c) >= 0) {
            return false;
        }
        int type = Character.getType(c);
        return type == Character.DASH_PUNCTUATION
                || type == Character.START_PUNCTUATION
                || type == Character.END_PUNCTUATION
                || type == Character.INITIAL_QUOTE_PUNCTUATION
                || type == Character.FINAL_QUOTE_PUNCTUATION
                || type == Character.CONNECTOR_PUNCTUATION
                || type == Character.OTHER_PUNCTUATION;
    }

    public static boolean isApostrophe(char c) {
        return c == '\'' || c == '’' || c == 'ʼ';
    }

    /**
     * Знаходить кінець багатосимвольного розділового знака, що починається в позиції start.
     * Групуються послідовності з ".!?" ("...", "?!", "!!") та дефісів ("--", "---").
     * Повертає індекс першого символу після групи.
     */
    public static int punctuationGroupEnd(String text, int start) {
        char first = text.charAt(start);
        int end = start + 1;
        if (SENTENCE_END_CHARS.indexOf(first) >= 0) {
            while (end < text.length() && SENTENCE_END_CHARS.indexOf(text.charAt(end)) >= 0) {
                end++;
            }
        } else if (first == '-') {
            while (end < text.length() && text.charAt(end) == '-') {
                end++;
            }
        }
        return end;
    }

    /**
     * Визначає тип довільного фрагмента тексту (використовується Whitespace токенізатором,
     * де токен може містити символи різних класів, наприклад "Hello,").
     */
    public static TokenType classify(String text) {
        boolean hasLetter = false;
        boolean hasDigit = false;
        boolean allPunctuation = true;
        boolean hasSpecial = false;

        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (Character.isLetter(c)) {
                hasLetter = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            }
            if (!isPunctuation(c)) {
                allPunctuation = false;
            }
            if (isSpecial(c)) {
                hasSpecial = true;
            }
        }

        if (text.isEmpty()) {
            return TokenType.UNKNOWN;
        }
        if (hasLetter) {
            return TokenType.WORD;
        }
        if (hasDigit) {
            return TokenType.NUMBER;
        }
        if (allPunctuation) {
            return TokenType.PUNCTUATION;
        }
        if (hasSpecial) {
            return TokenType.SPECIAL;
        }
        return TokenType.UNKNOWN;
    }
}
