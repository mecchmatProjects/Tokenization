package tokenization.tokenizer;

import tokenization.core.Token;

import java.util.ArrayList;
import java.util.List;

/**
 * Стратегія 3: токенізація на основі набору правил (README §11).
 */
public class RuleBasedTokenizer implements Tokenizer {

    @Override
    public List<Token> tokenize(String text) {
        // TODO: правила для чисел, email, URL, дат, скорочень, дефісів та апострофів
        return new ArrayList<>();
    }

    @Override
    public String getName() {
        return "Rule-based";
    }
}
