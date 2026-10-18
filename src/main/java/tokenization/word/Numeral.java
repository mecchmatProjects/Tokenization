package tokenization.word;

/**
 * ЧИСЛІВНИК.
 */
public class Numeral extends Word {

    public Numeral() {
        super(PartOfSpeech.NUMERAL);
        gender = Gender.MASCULINE;
        grammaticalCase = GrammaticalCase.NOMINATIVE;
    }

    public Numeral(String root, String ending) {
        this();
        addRoot(root);
        setEnding(ending);
    }

    // TODO: changeForm — порядкові числівники (перший, другий) відмінюються як прикметники
}
