package tokenization.core;

/**
 * Один токен тексту.
 */
public class Token {

    private final String text;
    private final TokenType type;
    // позиція першого символу токена в тексті
    private final int position;
    private int sentenceNumber;
    // -1 означає, що ID ще не призначено словником
    private int tokenId = -1;

    public Token(String text, TokenType type, int position) {
        this.text = text;
        this.type = type;
        this.position = position;
    }

    public String getText() {
        return text;
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
