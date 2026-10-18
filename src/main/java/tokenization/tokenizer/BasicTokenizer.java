package tokenization.tokenizer;

import tokenization.core.Token;

import java.util.ArrayList;
import java.util.List;

/**
 * Стратегія 2: відокремлює слова від розділових знаків (README §11).
 */
public class BasicTokenizer implements Tokenizer {

    @Override
    public List<Token> tokenize(String text) {
        // TODO: посимвольний прохід, окремі токени для слів, чисел, пунктуації та спецсимволів
        return new ArrayList<>();
    }

    @Override
    public String getName() {
        return "Basic";
    }
}
