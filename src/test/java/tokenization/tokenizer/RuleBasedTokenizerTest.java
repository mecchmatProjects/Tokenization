package tokenization.tokenizer;

import org.junit.jupiter.api.Test;
import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class RuleBasedTokenizerTest {

    private final RuleBasedTokenizer tokenizer = new RuleBasedTokenizer();

    private List<String> texts(String input) {
        List<String> result = new ArrayList<>();
        for (Token token : tokenizer.tokenize(input)) {
            result.add(token.getText());
        }
        return result;
    }

    private TokenType singleType(String input) {
        List<Token> tokens = tokenizer.tokenize(input);
        assertEquals(1, tokens.size(), "expected one token for: " + input);
        return tokens.get(0).getType();
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
    void recognizesNumbers() {
        assertEquals(List.of("Price", ":", "12.50"), texts("Price: 12.50"));
        assertEquals(TokenType.NUMBER, singleType("3.14"));
        assertEquals(TokenType.NUMBER, singleType("-15"));
        assertEquals(TokenType.NUMBER, singleType("1,000"));
        assertEquals(TokenType.NUMBER, singleType("15,5"));
    }

    @Test
    void negativeNumberInSentence() {
        assertEquals(List.of("The", "temperature", "is", "-12.5", "C", "."),
                texts("The temperature is -12.5 C."));
    }

    @Test
    void minusBetweenNumbersIsNotASign() {
        assertEquals(List.of("5", "-", "3"), texts("5-3"));
    }

    @Test
    void currencyAndPercentAreSeparate() {
        assertEquals(List.of("10", "%", "$", "100"), texts("10% $100"));
        assertEquals(List.of("price", "=", "100", "$"), texts("price=100$"));
    }

    @Test
    void recognizesEmail() {
        assertEquals(TokenType.EMAIL, singleType("user@example.com"));
        assertEquals(List.of("Contact", "me", "at", "test@example.com", "."),
                texts("Contact me at test@example.com."));
    }

    @Test
    void recognizesUrl() {
        assertEquals(TokenType.URL, singleType("https://example.com"));
        assertEquals(List.of("Visit", "https://example.com/page", "today", "!"),
                texts("Visit https://example.com/page today!"));
        assertEquals(List.of("(", "www.example.com", ")"), texts("(www.example.com)"));
    }

    @Test
    void recognizesDates() {
        assertEquals(TokenType.DATE, singleType("12.05.2024"));
        assertEquals(TokenType.DATE, singleType("2024-05-12"));
        assertEquals(TokenType.DATE, singleType("1/5/2024"));
    }

    @Test
    void keepsHyphenatedWords() {
        assertEquals(TokenType.WORD, singleType("machine-learning"));
        assertEquals(TokenType.WORD, singleType("mother-in-law"));
        assertEquals(TokenType.WORD, singleType("COVID-19"));
        assertEquals(TokenType.WORD, singleType("19-річний"));
    }

    @Test
    void keepsApostrophes() {
        assertEquals(List.of("Don't", "stop", "!"), texts("Don't stop!"));
        assertEquals(List.of("м'ята", ",", "п’ять"), texts("м'ята, п’ять"));
    }

    @Test
    void keepsAbbreviations() {
        assertEquals(List.of("e.g.", "U.S.A.", "Dr.", "Smith"), texts("e.g. U.S.A. Dr. Smith"));
        assertEquals(List.of("і", "т.д."), texts("і т.д."));
    }

    @Test
    void keepsCPlusPlus() {
        assertEquals(List.of("C++", "and", "Java"), texts("C++ and Java"));
        assertEquals(List.of("a", "+", "b"), texts("a+b"));
    }

    @Test
    void groupsMultiCharPunctuation() {
        assertEquals(List.of("Really", "?!", "Wait", "..."), texts("Really?! Wait..."));
        tokenizer.setGroupPunctuation(false);
        assertEquals(List.of("Wait", ".", ".", "."), texts("Wait..."));
    }

    @Test
    void wordsWithDigits() {
        assertEquals(List.of("2nd", "3D"), texts("2nd 3D"));
    }

    @Test
    void countsMatchReadmeTable() {
        // README §26: Rule-based дає 2, 1, 1, 1 токени
        assertEquals(2, texts("Hello,").size());
        assertEquals(1, texts("test@example.com").size());
        assertEquals(1, texts("19.99").size());
        assertEquals(1, texts("C++").size());
    }

    @Test
    void handlesUnicode() {
        assertEquals(List.of("Привіт", "світе", "!"), texts("Привіт світе!"));
        assertEquals(List.of("Bonjour", "le", "monde", "!"), texts("Bonjour le monde!"));
        assertEquals(List.of("你好世界", "！"), texts("你好世界！"));
        assertEquals(List.of("будь-який"), texts("будь-який"));
    }

    @Test
    void storesPositions() {
        List<Token> tokens = tokenizer.tokenize("Hi, Bob");
        assertEquals(0, tokens.get(0).getPosition());
        assertEquals(2, tokens.get(1).getPosition());
        assertEquals(4, tokens.get(2).getPosition());
    }

    @Test
    void emptyTextGivesNoTokens() {
        assertTrue(tokenizer.tokenize("").isEmpty());
        assertTrue(tokenizer.tokenize(" \n ").isEmpty());
    }
}
