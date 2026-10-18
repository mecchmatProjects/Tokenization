package tokenization.word;

import java.util.Scanner;

/**
 * ДІЄСЛОВО. Дієслово змінюється не за відмінками, а за особами та числами.
 *
 * Реалізовано теперішній час:
 * I дієвідміна з основою на голосний: чита-ю, чита-єш, чита-є, чита-ємо, чита-єте, чита-ють;
 * II дієвідміна: говор-ю, говор-иш, говор-ить, говор-имо, говор-ите, говор-ять.
 */
public class Verb extends Word {

    // закінчення в порядку: 1-ша, 2-га, 3-тя особа
    private static final String[] FIRST_SINGULAR = {"ю", "єш", "є"};
    private static final String[] FIRST_PLURAL = {"ємо", "єте", "ють"};
    private static final String[] SECOND_SINGULAR = {"ю", "иш", "ить"};
    private static final String[] SECOND_PLURAL = {"имо", "ите", "ять"};

    private int conjugation = 1;

    public Verb() {
        super(PartOfSpeech.VERB);
        person = Person.THIRD;
    }

    /** Дієслово в 3-й особі однини, напр. new Verb("чита", 1) → "читає", new Verb("говор", 2) → "говорить". */
    public Verb(String root, int conjugation) {
        this();
        addRoot(root);
        setConjugation(conjugation);
        changeForm(GrammaticalNumber.SINGULAR, Person.THIRD);
    }

    /** Дієслова не відмінюються. */
    @Override
    public void changeForm(GrammaticalNumber newNumber, GrammaticalCase newCase) {
        throw new UnsupportedOperationException("A verb changes by person and number, not by case");
    }

    public void changeForm(GrammaticalNumber newNumber, Person newPerson) {
        if (newPerson == Person.NONE) {
            throw new IllegalArgumentException("Person is required for a verb");
        }
        String[] table;
        if (conjugation == 1) {
            table = newNumber == GrammaticalNumber.SINGULAR ? FIRST_SINGULAR : FIRST_PLURAL;
        } else {
            table = newNumber == GrammaticalNumber.SINGULAR ? SECOND_SINGULAR : SECOND_PLURAL;
        }
        setEnding(table[newPerson.ordinal()]);
        number = newNumber;
        person = newPerson;
    }

    @Override
    public void readFromConsole(Scanner scanner) {
        super.readFromConsole(scanner);
        System.out.print("Дієвідміна (1 або 2) [" + conjugation + "]: ");
        String line = scanner.nextLine().trim();
        if (line.equals("1") || line.equals("2")) {
            conjugation = Integer.parseInt(line);
        }
    }

    @Override
    protected String getExtraField() {
        return String.valueOf(conjugation);
    }

    @Override
    protected void setExtraField(String value) {
        if (!value.isEmpty()) {
            setConjugation(Integer.parseInt(value));
        }
    }

    public int getConjugation() {
        return conjugation;
    }

    public void setConjugation(int conjugation) {
        if (conjugation != 1 && conjugation != 2) {
            throw new IllegalArgumentException("Conjugation must be 1 or 2");
        }
        this.conjugation = conjugation;
    }
}
