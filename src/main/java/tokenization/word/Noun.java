package tokenization.word;

/**
 * ІМЕННИК.
 */
public class Noun extends Word {

    public Noun() {
        super(PartOfSpeech.NOUN);
        gender = Gender.MASCULINE;
        grammaticalCase = GrammaticalCase.NOMINATIVE;
    }

    /** Іменник у називному відмінку однини, напр. new Noun("книг", "а", Gender.FEMININE). */
    public Noun(String root, String ending, Gender gender) {
        this();
        addRoot(root);
        setEnding(ending);
        this.gender = gender;
    }

    // TODO: changeForm — відмінювання за таблицями закінчень для кожного роду
}
