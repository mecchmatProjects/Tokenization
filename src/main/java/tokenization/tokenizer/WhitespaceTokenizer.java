package tokenization.tokenizer;

import tokenization.core.Token;

import java.util.ArrayList;
import java.util.List;

/**
 * Стратегія 1: розбиття тексту лише за пробілами (README §11).
 */
public class WhitespaceTokenizer implements Tokenizer {

    @Override
    public List<Token> tokenize(String text) {
        // TODO: розбити текст за пробільними символами та визначити тип кожного фрагмента
        return new ArrayList<>();
    }

    @Override
    public String getName() {
        return "Whitespace";
    }
}
