package tokenization.normalization;

import org.junit.jupiter.api.Test;
import tokenization.core.Token;
import tokenization.tokenizer.RuleBasedTokenizer;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class NormalizerTest {

    private final RuleBasedTokenizer tokenizer = new RuleBasedTokenizer();

    private String normalize(Normalizer normalizer, String text) {
        List<Token> tokens = normalizer.normalize(tokenizer.tokenize(text));
        return String.join(" ", Normalizer.texts(tokens));
    }

    @Test
    void lowercaseMergesDifferentCases() {
        Normalizer normalizer = new Normalizer();
        assertEquals("machine machine machine", normalize(normalizer, "Machine machine MACHINE"));
    }

    @Test
    void caseSensitiveModeKeepsOriginalCase() {
        Normalizer normalizer = new Normalizer();
        normalizer.setLowercase(false);
        assertEquals("Machine machine MACHINE", normalize(normalizer, "Machine machine MACHINE"));
    }

    @Test
    void keepsOriginalText() {
        Normalizer normalizer = new Normalizer();
        List<Token> tokens = normalizer.normalize(tokenizer.tokenize("Hello"));
        assertEquals("Hello", tokens.get(0).getText());
        assertEquals("hello", tokens.get(0).getNormalizedText());
    }

    @Test
    void replacesUrlAsInReadmeExample() {
        Normalizer normalizer = new Normalizer();
        normalizer.setReplaceUrls(true);
        assertEquals("visit <URL> today !", normalize(normalizer, "Visit https://example.com today!"));
    }

    @Test
    void replacesEmailsAndNumbers() {
        Normalizer normalizer = new Normalizer();
        normalizer.setReplaceEmails(true);
        normalizer.setReplaceNumbers(true);
        assertEquals("write <EMAIL> <NUM> times", normalize(normalizer, "Write a@b.com 2 times"));
    }

    @Test
    void removesPunctuation() {
        Normalizer normalizer = new Normalizer();
        normalizer.setRemovePunctuation(true);
        assertEquals("hello world", normalize(normalizer, "Hello, world!"));
    }

    @Test
    void removesStopWordsAsInReadmeExample() {
        Normalizer normalizer = new Normalizer();
        normalizer.setRemoveStopWords(true);
        normalizer.setRemovePunctuation(true);
        assertEquals("machine learning algorithm very fast",
                normalize(normalizer, "The machine learning algorithm is very fast."));
    }

    @Test
    void removesUkrainianStopWords() {
        Normalizer normalizer = new Normalizer();
        normalizer.setRemoveStopWords(true);
        assertEquals("кіт , пес", normalize(normalizer, "І кіт, і пес"));
    }

    @Test
    void emptyInputGivesEmptyOutput() {
        assertTrue(new Normalizer().normalize(tokenizer.tokenize("")).isEmpty());
    }
}
