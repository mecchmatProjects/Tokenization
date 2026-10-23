package tokenization.tokenizer;

import org.junit.jupiter.api.Test;
import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class WhitespaceTokenizerTest {

    private final Tokenizer tokenizer = new WhitespaceTokenizer();

    private List<String> texts(String input) {
        List<String> result = new ArrayList<>();
        for (Token token : tokenizer.tokenize(input)) {
            result.add(token.getText());
        }
        return result;
    }

    @Test
    void splitsSimpleText() {
        assertEquals(List.of("Hello", "world"), texts("Hello world"));
    }

    @Test
    void keepsPunctuationAttached() {
        assertEquals(List.of("Hello,", "world!"), texts("Hello, world!"));
    }

    @Test
    void handlesSeveralWhitespaceCharacters() {
        assertEquals(List.of("a", "b", "c"), texts("  a \t\n b   c  "));
    }

    @Test
    void storesPositionsAndTypes() {
        List<Token> tokens = tokenizer.tokenize("Price: 12.50 !");
        assertEquals(0, tokens.get(0).getPosition());
        assertEquals(7, tokens.get(1).getPosition());
        assertEquals(TokenType.WORD, tokens.get(0).getType());
        assertEquals(TokenType.NUMBER, tokens.get(1).getType());
        assertEquals(TokenType.PUNCTUATION, tokens.get(2).getType());
    }

    @Test
    void emailIsOneToken() {
        assertEquals(List.of("test@example.com"), texts("test@example.com"));
    }

    @Test
    void emptyTextGivesNoTokens() {
        assertTrue(tokenizer.tokenize("").isEmpty());
        assertTrue(tokenizer.tokenize("   ").isEmpty());
    }
}
