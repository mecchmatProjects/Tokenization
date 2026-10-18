package tokenization.word;

import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

class InflectionTest {

    /** Усі сім відмінків слова в заданому числі. */
    private List<String> allCases(Word word, GrammaticalNumber number) {
        List<String> forms = new ArrayList<>();
        for (GrammaticalCase grammaticalCase : GrammaticalCase.values()) {
            if (grammaticalCase != GrammaticalCase.NONE) {
                word.changeForm(number, grammaticalCase);
                forms.add(word.getText());
            }
        }
        return forms;
    }

    @Test
    void declinesFeminineNoun() {
        Noun noun = new Noun("хат", "а", Gender.FEMININE);
        assertEquals(List.of("хата", "хати", "хаті", "хату", "хатою", "хаті", "хато"),
                allCases(noun, GrammaticalNumber.SINGULAR));
        assertEquals(List.of("хати", "хат", "хатам", "хати", "хатами", "хатах", "хати"),
                allCases(noun, GrammaticalNumber.PLURAL));
    }

    @Test
    void declinesMasculineNounDependingOnAnimacy() {
        Noun factory = new Noun("завод", "", Gender.MASCULINE);
        assertEquals(List.of("завод", "заводу", "заводові", "завод", "заводом", "заводі", "заводе"),
                allCases(factory, GrammaticalNumber.SINGULAR));

        Noun student = new Noun("студент", "", Gender.MASCULINE);
        student.setAnimate(true);
        assertEquals(List.of("студент", "студента", "студентові", "студента", "студентом", "студенті", "студенте"),
                allCases(student, GrammaticalNumber.SINGULAR));
        assertEquals(List.of("студенти", "студентів", "студентам", "студентів", "студентами", "студентах", "студенти"),
                allCases(student, GrammaticalNumber.PLURAL));
    }

    @Test
    void declinesNeuterNoun() {
        Noun noun = new Noun("міст", "о", Gender.NEUTER);
        assertEquals(List.of("міста", "міст", "містам", "міста", "містами", "містах", "міста"),
                allCases(noun, GrammaticalNumber.PLURAL));
    }

    @Test
    void changeFormUpdatesProperties() {
        Noun noun = new Noun("хат", "а", Gender.FEMININE);
        noun.changeForm(GrammaticalNumber.PLURAL, GrammaticalCase.INSTRUMENTAL);
        assertEquals(GrammaticalNumber.PLURAL, noun.getNumber());
        assertEquals(GrammaticalCase.INSTRUMENTAL, noun.getGrammaticalCase());
        assertEquals("ами", noun.getEnding());
    }

    @Test
    void declinesAdjectiveInAllGenders() {
        Adjective adjective = new Adjective("червон", "ий");
        assertEquals(List.of("червоний", "червоного", "червоному", "червоний", "червоним", "червоному", "червоний"),
                allCases(adjective, GrammaticalNumber.SINGULAR));

        adjective.changeForm(Gender.FEMININE, GrammaticalNumber.SINGULAR, GrammaticalCase.GENITIVE);
        assertEquals("червоної", adjective.getText());
        adjective.changeForm(Gender.NEUTER, GrammaticalNumber.SINGULAR, GrammaticalCase.NOMINATIVE);
        assertEquals("червоне", adjective.getText());
        adjective.changeForm(GrammaticalNumber.PLURAL, GrammaticalCase.INSTRUMENTAL);
        assertEquals("червоними", adjective.getText());
    }

    @Test
    void conjugatesVerbs() {
        Verb read = new Verb("чита", 1);
        assertEquals("читає", read.getText());
        read.changeForm(GrammaticalNumber.SINGULAR, Person.FIRST);
        assertEquals("читаю", read.getText());
        read.changeForm(GrammaticalNumber.PLURAL, Person.THIRD);
        assertEquals("читають", read.getText());

        Verb speak = new Verb("говор", 2);
        speak.changeForm(GrammaticalNumber.SINGULAR, Person.SECOND);
        assertEquals("говориш", speak.getText());
        speak.changeForm(GrammaticalNumber.PLURAL, Person.FIRST);
        assertEquals("говоримо", speak.getText());

        assertThrows(UnsupportedOperationException.class,
                () -> speak.changeForm(GrammaticalNumber.SINGULAR, GrammaticalCase.GENITIVE));
    }

    @Test
    void declinesOrdinalNumeralLikeAdjective() {
        Numeral first = new Numeral("перш", "ий", true);
        first.changeForm(GrammaticalNumber.SINGULAR, GrammaticalCase.DATIVE);
        assertEquals("першому", first.getText());
        first.changeForm(Gender.FEMININE, GrammaticalNumber.SINGULAR, GrammaticalCase.NOMINATIVE);
        assertEquals("перша", first.getText());

        Numeral five = new Numeral("п'ять", "", false);
        assertThrows(UnsupportedOperationException.class,
                () -> five.changeForm(GrammaticalNumber.SINGULAR, GrammaticalCase.GENITIVE));
    }

    @Test
    void otherWordsCannotChangeForm() {
        Word adverb = new Word(PartOfSpeech.ADVERB);
        adverb.addRoot("швидко");
        assertThrows(UnsupportedOperationException.class,
                () -> adverb.changeForm(GrammaticalNumber.PLURAL, GrammaticalCase.GENITIVE));
    }

    @Test
    void subclassPropertiesSurviveFileRoundTrip() {
        Noun student = new Noun("студент", "", Gender.MASCULINE);
        student.setAnimate(true);
        Word loaded = Word.fromFileLine(student.toFileLine());
        assertInstanceOf(Noun.class, loaded);
        assertTrue(((Noun) loaded).isAnimate());

        Verb verb = new Verb("говор", 2);
        assertEquals(2, ((Verb) Word.fromFileLine(verb.toFileLine())).getConjugation());

        Numeral numeral = new Numeral("друг", "ий", true);
        assertTrue(((Numeral) Word.fromFileLine(numeral.toFileLine())).isOrdinal());
    }
}
