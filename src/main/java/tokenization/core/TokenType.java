package tokenization.core;

/**
 * Тип токена (README §6).
 */
public enum TokenType {
    WORD,
    NUMBER,
    PUNCTUATION,
    SPECIAL,
    WHITESPACE,
    UNKNOWN,

    // типи, які розпізнає лише Rule-based токенізатор (README §11, стратегія 3)
    URL,
    EMAIL,
    DATE
}
