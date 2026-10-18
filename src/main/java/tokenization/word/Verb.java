package tokenization.word;

/**
 * ДІЄСЛОВО. Дієслово змінюється не за відмінками, а за особами та числами.
 */
public class Verb extends Word {

    public Verb() {
        super(PartOfSpeech.VERB);
        person = Person.THIRD;
    }

    /** Дієслово в 3-й особі однини теперішнього часу, напр. new Verb("чита", "є"). */
    public Verb(String root, String ending) {
        this();
        addRoot(root);
        setEnding(ending);
    }

    // TODO: changeForm(число, особа) — дієвідмінювання в теперішньому часі
}
