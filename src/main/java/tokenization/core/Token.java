package tokenization.core;

/**
 * Один токен тексту.
 */
public class Token {

    private final String text;
    // текст після нормалізації (нижній регістр, <URL> тощо); спочатку збігається з text
    private String normalizedText;
    // базова форма слова; null, якщо лематизація не виконувалась
    private String lemma;
    private final TokenType type;
    // позиція першого символу токена в тексті
    private final int position;
    private int sentenceNumber;
    // -1 означає, що ID ще не призначено словником
    private int tokenId = -1;

    public Token(String text, TokenType type, int position) {
        this.text = text;
        this.normalizedText = text;
        this.type = type;
        this.position = position;
    }

    public String getText() {
        return text;
    }

    public String getNormalizedText() {
        return normalizedText;
    }

    public void setNormalizedText(String normalizedText) {
        this.normalizedText = normalizedText;
    }

    /** Лема токена, або нормалізований текст, якщо лему ще не визначено. */
    public String getLemma() {
        return lemma != null ? lemma : normalizedText;
    }

    public void setLemma(String lemma) {
        this.lemma = lemma;
    }

    public TokenType getType() {
        return type;
    }

    public int getPosition() {
        return position;
    }

    public int getSentenceNumber() {
        return sentenceNumber;
    }

    public void setSentenceNumber(int sentenceNumber) {
        this.sentenceNumber = sentenceNumber;
    }

    public int getTokenId() {
        return tokenId;
    }

    public void setTokenId(int tokenId) {
        this.tokenId = tokenId;
    }

    @Override
    public String toString() {
        return text + " [" + type + "]";
    }
}
