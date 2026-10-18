package tokenization.normalization;

import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

/**
 * Нормалізація токенів після токенізації (README §13).
 *
 * Результат записується в normalizedText кожного токена, оригінальний text не змінюється.
 * Частина токенів може бути видалена (пунктуація, stop words, пробіли).
 */
public class Normalizer {

    public static final String NUMBER_TOKEN = "<NUM>";
    public static final String URL_TOKEN = "<URL>";
    public static final String EMAIL_TOKEN = "<EMAIL>";

    private boolean lowercase = true;
    private boolean removePunctuation = false;
    private boolean replaceNumbers = false;
    private boolean replaceUrls = false;
    private boolean replaceEmails = false;
    private boolean removeStopWords = false;
    private boolean lemmatize = false;

    private StopWords stopWords;
    private Lemmatizer lemmatizer;

    public List<Token> normalize(List<Token> tokens) {
        List<Token> result = new ArrayList<>();

        for (Token token : tokens) {
            TokenType type = token.getType();
            if (type == TokenType.WHITESPACE) {
                continue;
            }
            if (removePunctuation && type == TokenType.PUNCTUATION) {
                continue;
            }
            if (removeStopWords && type == TokenType.WORD && getStopWords().isStopWord(token.getText())) {
                continue;
            }

            String value = token.getText();
            if (lowercase) {
                value = value.toLowerCase();
            }
            if (replaceNumbers && type == TokenType.NUMBER) {
                value = NUMBER_TOKEN;
            } else if (replaceUrls && type == TokenType.URL) {
                value = URL_TOKEN;
            } else if (replaceEmails && type == TokenType.EMAIL) {
                value = EMAIL_TOKEN;
            }
            if (lemmatize && type == TokenType.WORD) {
                value = getLemmatizer().lemmatize(value);
                token.setLemma(value);
            }

            token.setNormalizedText(value);
            result.add(token);
        }
        return result;
    }

    /** Нормалізовані тексти токенів, наприклад для побудови словника. */
    public static List<String> texts(List<Token> tokens) {
        List<String> result = new ArrayList<>();
        for (Token token : tokens) {
            result.add(token.getNormalizedText());
        }
        return result;
    }

    // словники завантажуються лише тоді, коли вони справді потрібні
    private StopWords getStopWords() {
        if (stopWords == null) {
            stopWords = StopWords.loadDefault();
        }
        return stopWords;
    }

    private Lemmatizer getLemmatizer() {
        if (lemmatizer == null) {
            lemmatizer = new Lemmatizer();
        }
        return lemmatizer;
    }

    public void setStopWords(StopWords stopWords) {
        this.stopWords = stopWords;
    }

    public boolean isLowercase() {
        return lowercase;
    }

    public void setLowercase(boolean lowercase) {
        this.lowercase = lowercase;
    }

    public boolean isRemovePunctuation() {
        return removePunctuation;
    }

    public void setRemovePunctuation(boolean removePunctuation) {
        this.removePunctuation = removePunctuation;
    }

    public boolean isReplaceNumbers() {
        return replaceNumbers;
    }

    public void setReplaceNumbers(boolean replaceNumbers) {
        this.replaceNumbers = replaceNumbers;
    }

    public boolean isReplaceUrls() {
        return replaceUrls;
    }

    public void setReplaceUrls(boolean replaceUrls) {
        this.replaceUrls = replaceUrls;
    }

    public boolean isReplaceEmails() {
        return replaceEmails;
    }

    public void setReplaceEmails(boolean replaceEmails) {
        this.replaceEmails = replaceEmails;
    }

    public boolean isRemoveStopWords() {
        return removeStopWords;
    }

    public void setRemoveStopWords(boolean removeStopWords) {
        this.removeStopWords = removeStopWords;
    }

    public boolean isLemmatize() {
        return lemmatize;
    }

    public void setLemmatize(boolean lemmatize) {
        this.lemmatize = lemmatize;
    }
}
