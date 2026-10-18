package tokenization.word;

/**
 * ПРИКМЕТНИК.
 */
public class Adjective extends Word {

    public Adjective() {
        super(PartOfSpeech.ADJECTIVE);
        gender = Gender.MASCULINE;
        grammaticalCase = GrammaticalCase.NOMINATIVE;
    }

    /** Прикметник у називному відмінку однини чоловічого роду, напр. new Adjective("червон", "ий"). */
    public Adjective(String root, String ending) {
        this();
        addRoot(root);
        setEnding(ending);
    }

    // TODO: changeForm — відмінювання з урахуванням роду (тверда група: -ий, -а, -е)
}
