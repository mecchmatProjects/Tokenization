package tokenization.word;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class HyphenatorTest {

    private final Hyphenator hyphenator = new Hyphenator();

    private String split(String word) {
        return String.join(" ", hyphenator.split(word));
    }

    @Test
    void rule1SplitsTwoVowels() {
        assertEquals("по ет", split("поет"));
        assertEquals("за єць", split("заєць"));
    }

    @Test
    void rule2SplitsTwoConsonants() {
        assertEquals("сес тра", split("сестра"));
        assertEquals("мор ква", split("морква"));
    }

    @Test
    void rule3SplitsAfterVowel() {
        assertEquals("ма ма", split("мама"));
        assertEquals("мо ло ко", split("молоко"));
        assertEquals("уні вер си тет", split("університет"));
    }

    @Test
    void shortYIsJoinedWithPreviousVowel() {
        assertEquals("лій ка", split("лійка"));
    }

    @Test
    void softSignAndApostropheStayWithConsonant() {
        assertEquals("м'я та", split("м'ята"));
        assertEquals("п'ять", split("п'ять"));
        assertEquals("сі м'я", split("сім'я"));
        assertEquals("кіль ка", split("кілька"));
    }

    @Test
    void prefixIsSeparatedFirst() {
        assertEquals("при лі сок", split("прилісок"));
        assertEquals("до ро га", split("дорога"));
        assertEquals("пе ре біг ти", split("перебігти"));
    }

    @Test
    void keepsCaseAndHyphen() {
        assertEquals("Ук ра їна", split("Україна"));
        assertEquals("будь- який", split("будь-який"));
    }

    @Test
    void oneSyllableWordsAreNotSplit() {
        assertEquals("кіт", split("кіт"));
        assertEquals("день", split("день"));
        assertTrue(hyphenator.split("").isEmpty());
    }

    @Test
    void countsSyllablesByVowels() {
        assertEquals(5, Hyphenator.countSyllables("університет"));
        assertEquals(1, Hyphenator.countSyllables("п'ять"));
        assertEquals(0, Hyphenator.countSyllables("hello"));
    }

    @Test
    void wordSplitsAndMergesSyllables() {
        Word word = new Noun();
        word.addPrefix("при");
        word.addRoot("ліс");
        word.addSuffix("ок");

        List<String> syllables = word.splitIntoSyllables();
        assertEquals(List.of("при", "лі", "сок"), syllables);
        assertEquals("при-лі-сок", word.syllablesToString());
        assertEquals("прилісок", Word.mergeSyllables(syllables));
    }
}
