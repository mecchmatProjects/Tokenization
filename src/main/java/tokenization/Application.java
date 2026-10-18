package tokenization;

import tokenization.core.Token;
import tokenization.preprocessing.TextPreprocessor;
import tokenization.tokenizer.BasicTokenizer;
import tokenization.tokenizer.RuleBasedTokenizer;
import tokenization.tokenizer.Tokenizer;
import tokenization.tokenizer.WhitespaceTokenizer;

import java.util.List;

/**
 * Точка входу в програму.
 */
public class Application {

    public static void main(String[] args) {
        System.out.println("=== Text Tokenizer ===");

        // TODO: замінити демонстрацію консольним меню (README §27)
        String text = "Machine learning is amazing! I have 2 projects.";
        TextPreprocessor preprocessor = new TextPreprocessor();
        String processed = preprocessor.process(text);

        Tokenizer[] tokenizers = {
                new WhitespaceTokenizer(),
                new BasicTokenizer(),
                new RuleBasedTokenizer()
        };
        for (Tokenizer tokenizer : tokenizers) {
            List<Token> tokens = tokenizer.tokenize(processed);
            System.out.println(tokenizer.getName() + ": " + tokens);
        }

        // TODO: нормалізація (README §13-15)
        // TODO: словник і числове представлення (README §16-23)
        // TODO: статистика та експорт (README §25, §28)
    }
}
