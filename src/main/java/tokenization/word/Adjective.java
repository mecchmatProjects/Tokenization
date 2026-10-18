package tokenization.word;

/**
 * ПРИКМЕТНИК. Відмінювання реалізовано для твердої групи (червоний, червона, червоне).
 */
public class Adjective extends Word {

    // закінчення в порядку відмінків: Н, Р, Д, З, О, М, К
    private static final String[] MASCULINE = {"ий", "ого", "ому", "ий", "им", "ому", "ий"};
    private static final String[] FEMININE = {"а", "ої", "ій", "у", "ою", "ій", "а"};
    private static final String[] NEUTER = {"е", "ого", "ому", "е", "им", "ому", "е"};
    private static final String[] PLURAL = {"і", "их", "им", "і", "ими", "их", "і"};

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

    @Override
    public void changeForm(GrammaticalNumber newNumber, GrammaticalCase newCase) {
        changeForm(gender, newNumber, newCase);
    }

    /** Прикметник, на відміну від іменника, змінюється ще й за родами. */
    public void changeForm(Gender newGender, GrammaticalNumber newNumber, GrammaticalCase newCase) {
        setEnding(hardEnding(newGender, newNumber, newCase));
        gender = newGender;
        number = newNumber;
        grammaticalCase = newCase;
    }

    /** Закінчення прикметника твердої групи. Використовується також порядковими числівниками. */
    static String hardEnding(Gender gender, GrammaticalNumber number, GrammaticalCase grammaticalCase) {
        if (grammaticalCase == GrammaticalCase.NONE) {
            throw new IllegalArgumentException("Case is required");
        }
        int index = grammaticalCase.ordinal();
        if (number == GrammaticalNumber.PLURAL) {
            return PLURAL[index];
        }
        if (gender == Gender.FEMININE) {
            return FEMININE[index];
        }
        if (gender == Gender.NEUTER) {
            return NEUTER[index];
        }
        return MASCULINE[index];
    }
}
