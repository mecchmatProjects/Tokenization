package tokenization.normalization;

import org.junit.jupiter.api.Test;
import tokenization.core.Token;
import tokenization.tokenizer.RuleBasedTokenizer;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class LemmatizerTest {

    private final Lemmatizer lemmatizer = new Lemmatizer();

    private Set<String> lemmas(String... words) {
        Set<String> result = new HashSet<>();
        for (String word : words) {
            result.add(lemmatizer.lemmatize(word));
        }
        return result;
    }

    @Test
    void reducesEnglishFormsToCommonStem() {
        // README §15
        assertEquals(Set.of("connect"), lemmas("connect", "connected", "connecting", "connection", "connects"));
        assertEquals("study", lemmatizer.lemmatize("studies"));
    }

    @Test
    void doesNotCutShortOrSpecialEnglishWords() {
        assertEquals("this", lemmatizer.lemmatize("this"));
        assertEquals("class", lemmatizer.lemmatize("class"));
    }

    @Test
    void usesDictionaryForIrregularEnglishWords() {
        assertEquals(Set.of("be"), lemmas("is", "are", "was", "were"));
        assertEquals("person", lemmatizer.lemmatize("people"));
    }

    @Test
    void usesDictionaryForUkrainianWords() {
        // README_LITCRITIC §6
        assertEquals(Set.of("будинок"), lemmas("будинок", "будинку", "будинком", "будинки", "будинків"));
        assertEquals(Set.of("йти"), lemmas("йшов", "йде", "йти", "йшли"));
        assertEquals("бути", lemmatizer.lemmatize("Була"));
    }

    @Test
    void stripsUkrainianEndings() {
        assertEquals(Set.of("книг"), lemmas("книга", "книги", "книгою", "книгами"));
        assertEquals(Set.of("червон"), lemmas("червоний", "червона", "червоного"));
    }

    @Test
    void keepsShortWords() {
        assertEquals("дім", lemmatizer.lemmatize("дім"));
        assertEquals("кіт", lemmatizer.lemmatize("кіт"));
    }

    @Test
    void normalizerUsesLemmatizerWhenEnabled() {
        Normalizer normalizer = new Normalizer();
        normalizer.setLemmatize(true);
        List<Token> tokens = normalizer.normalize(new RuleBasedTokenizer().tokenize("Connected connecting"));
        assertEquals(List.of("connect", "connect"), Normalizer.texts(tokens));
        assertEquals("connect", tokens.get(0).getLemma());
        assertTrue(lemmatizer.dictionarySize() > 0);
    }
}
