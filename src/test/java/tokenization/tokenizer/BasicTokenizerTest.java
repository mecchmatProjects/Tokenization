package tokenization.tokenizer;

import org.junit.jupiter.api.Test;
import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class BasicTokenizerTest {

    private final BasicTokenizer tokenizer = new BasicTokenizer();

    private List<String> texts(String input) {
        List<String> result = new ArrayList<>();
        for (Token token : tokenizer.tokenize(input)) {
            result.add(token.getText());
        }
        return result;
    }

    private List<TokenType> types(String input) {
        List<TokenType> result = new ArrayList<>();
        for (Token token : tokenizer.tokenize(input)) {
            result.add(token.getType());
        }
        return result;
    }

    @Test
    void splitsSimpleText() {
        assertEquals(List.of("Hello", "world"), texts("Hello world"));
    }

    @Test
    void separatesPunctuation() {
        assertEquals(List.of("Hello", ",", "world", "!"), texts("Hello, world!"));
    }

    @Test
    void detectsTypesFromReadmeExample() {
        // README §5: Hello, world! Price = 12.50$
        assertEquals(List.of("Hello", ",", "world", "!", "Price", "=", "12", ".", "50", "$"),
                texts("Hello, world! Price = 12.50$"));
        assertEquals(List.of(TokenType.WORD, TokenType.PUNCTUATION, TokenType.WORD, TokenType.PUNCTUATION,
                        TokenType.WORD, TokenType.SPECIAL, TokenType.NUMBER, TokenType.PUNCTUATION,
                        TokenType.NUMBER, TokenType.SPECIAL),
                types("Hello, world! Price = 12.50$"));
    }

    @Test
    void splitsConstructionsAsInReadmeTable() {
        // README §26: Basic дає 5, 3 та 3 токени
        assertEquals(5, texts("test@example.com").size());
        assertEquals(3, texts("19.99").size());
        assertEquals(3, texts("C++").size());
    }

    @Test
    void splitsApostrophesAndHyphens() {
        assertEquals(List.of("don", "'", "t"), texts("don't"));
        assertEquals(List.of("machine", "-", "learning"), texts("machine-learning"));
    }

    @Test
    void splitsMultiCharPunctuationByDefault() {
        assertEquals(List.of("Wait", ".", ".", "."), texts("Wait..."));
    }

    @Test
    void groupsMultiCharPunctuationWhenEnabled() {
        tokenizer.setGroupPunctuation(true);
        assertEquals(List.of("Really", "?!", "Yes", "...", "--"), texts("Really?! Yes... --"));
    }

    @Test
    void handlesUnicode() {
        assertEquals(List.of("Привіт", "світе", "!"), texts("Привіт світе!"));
        assertEquals(List.of("Bonjour", "le", "monde", "!"), texts("Bonjour le monde!"));
        assertEquals(List.of("你好世界", "！"), texts("你好世界！"));
    }

    @Test
    void keepsWhitespaceWhenEnabled() {
        tokenizer.setKeepWhitespace(true);
        List<Token> tokens = tokenizer.tokenize("a  b");
        assertEquals(3, tokens.size());
        assertEquals(TokenType.WHITESPACE, tokens.get(1).getType());
        assertEquals("  ", tokens.get(1).getText());
    }

    @Test
    void emptyTextGivesNoTokens() {
        assertTrue(tokenizer.tokenize("").isEmpty());
    }
}
