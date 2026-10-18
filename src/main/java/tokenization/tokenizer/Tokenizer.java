package tokenization.tokenizer;

import tokenization.core.Token;

import java.util.List;

/**
 * Спільний інтерфейс для всіх стратегій токенізації (README §12).
 */
public interface Tokenizer {

    List<Token> tokenize(String text);

    /** Назва стратегії для виведення в меню та звітах. */
    String getName();
}
