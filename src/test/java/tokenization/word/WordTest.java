package tokenization.word;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.nio.file.Path;
import java.util.List;
import java.util.Scanner;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertThrows;

class WordTest {

    @TempDir
    Path tempDir;

    private Word prylisok() {
        Word word = new Noun();
        word.addPrefix("при");
        word.addRoot("ліс");
        word.addSuffix("ок");
        return word;
    }

    @Test
    void buildsTextFromParts() {
        Word word = prylisok();
        assertEquals("прилісок", word.getText());
        assertEquals(List.of("при"), word.getPrefixes());
        assertEquals(List.of("ліс"), word.getRoots());
        assertEquals(List.of("ок"), word.getSuffixes());
        assertEquals("", word.getEnding());
    }

    @Test
    void showsParts() {
        assertEquals("префікс: при | корінь: ліс | суфікс: ок | закінчення: ∅", prylisok().partsToString());
    }

    @Test
    void supportsTwoRoots() {
        // з'єднувальний голосний записуємо разом з першим коренем
        Word word = new Noun();
        word.addRoot("лісо");
        word.addRoot("степ");
        assertEquals(List.of("лісо", "степ"), word.getRoots());
        assertEquals("лісостеп", word.getText());
    }

    @Test
    void limitsNumberOfParts() {
        Word word = new Word();
        word.addPrefix("пере");
        word.addPrefix("за");
        word.addPrefix("на");
        assertThrows(IllegalStateException.class, () -> word.addPrefix("по"));

        word.addRoot("а");
        word.addRoot("б");
        assertThrows(IllegalStateException.class, () -> word.addRoot("в"));
        assertThrows(IllegalArgumentException.class, () -> new Word().addRoot(""));
    }

    @Test
    void setsProperties() {
        Word word = new Noun("книг", "а", Gender.FEMININE);
        word.setProperties(Gender.FEMININE, GrammaticalNumber.PLURAL, GrammaticalCase.GENITIVE);
        assertEquals(Gender.FEMININE, word.getGender());
        assertEquals(GrammaticalNumber.PLURAL, word.getNumber());
        assertEquals(GrammaticalCase.GENITIVE, word.getGrammaticalCase());
    }

    @Test
    void createsSubclassByType() {
        assertInstanceOf(Noun.class, Word.create(PartOfSpeech.NOUN));
        assertInstanceOf(Verb.class, Word.create(PartOfSpeech.VERB));
        assertInstanceOf(Adjective.class, Word.create(PartOfSpeech.ADJECTIVE));
        assertInstanceOf(Numeral.class, Word.create(PartOfSpeech.NUMERAL));
        assertEquals(PartOfSpeech.ADVERB, Word.create(PartOfSpeech.ADVERB).getType());
    }

    @Test
    void savesAndLoadsFile() throws IOException {
        Word noun = prylisok();
        Word adjective = new Adjective("червон", "ий");
        Path file = tempDir.resolve("words.txt");

        Word.saveToFile(List.of(noun, adjective), file);
        List<Word> loaded = Word.loadFromFile(file);

        assertEquals(2, loaded.size());
        assertInstanceOf(Noun.class, loaded.get(0));
        assertEquals("прилісок", loaded.get(0).getText());
        assertEquals(noun.toFileLine(), loaded.get(0).toFileLine());
        assertEquals("червоний", loaded.get(1).getText());
    }

    @Test
    void fileLineFormat() {
        assertEquals("NOUN;при;ліс;ок;;MASCULINE;SINGULAR;NONE;NOMINATIVE", prylisok().toFileLine());
        assertThrows(IllegalArgumentException.class, () -> Word.fromFileLine("NOUN;ліс"));
    }

    @Test
    void readsFromConsole() {
        // префікси, корені, суфікси, закінчення, рід, число, особа, відмінок
        String input = "\nкниг\n\nа\n2\n\n\n1\n";
        Word word = new Noun();
        word.readFromConsole(new Scanner(input));

        assertEquals("книга", word.getText());
        assertEquals(Gender.FEMININE, word.getGender());
        assertEquals(GrammaticalNumber.SINGULAR, word.getNumber());
        assertEquals(GrammaticalCase.NOMINATIVE, word.getGrammaticalCase());
    }
}
