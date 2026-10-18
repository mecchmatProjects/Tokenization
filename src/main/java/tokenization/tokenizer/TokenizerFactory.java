package tokenization.tokenizer;

import java.util.ArrayList;
import java.util.List;

/**
 * Створення токенізатора за номером стратегії (1, 2, 3), як у меню.
 */
public class TokenizerFactory {

    public static final int WHITESPACE = 1;
    public static final int BASIC = 2;
    public static final int RULE_BASED = 3;

    private TokenizerFactory() {
    }

    public static Tokenizer create(int strategy, boolean groupPunctuation) {
        if (strategy == WHITESPACE) {
            return new WhitespaceTokenizer();
        }
        if (strategy == BASIC) {
            BasicTokenizer tokenizer = new BasicTokenizer();
            tokenizer.setGroupPunctuation(groupPunctuation);
            return tokenizer;
        }
        if (strategy == RULE_BASED) {
            RuleBasedTokenizer tokenizer = new RuleBasedTokenizer();
            tokenizer.setGroupPunctuation(groupPunctuation);
            return tokenizer;
        }
        throw new IllegalArgumentException("Unknown strategy: " + strategy);
    }

    /** Усі три стратегії для порівняння. */
    public static List<Tokenizer> createAll(boolean groupPunctuation) {
        List<Tokenizer> tokenizers = new ArrayList<>();
        tokenizers.add(create(WHITESPACE, groupPunctuation));
        tokenizers.add(create(BASIC, groupPunctuation));
        tokenizers.add(create(RULE_BASED, groupPunctuation));
        return tokenizers;
    }
}
