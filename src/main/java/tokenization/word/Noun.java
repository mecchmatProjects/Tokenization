package tokenization.word;

import java.util.Scanner;

/**
 * ІМЕННИК.
 *
 * Відмінювання реалізовано для твердої групи: жіночий рід на -а (хата),
 * чоловічий на приголосний (завод, студент), середній на -о (місто).
 * Чергування звуків (книга — книзі, стіл — стола) не враховується.
 */
public class Noun extends Word {

    // закінчення в порядку відмінків: Н, Р, Д, З, О, М, К
    private static final String[] FEMININE_SINGULAR = {"а", "и", "і", "у", "ою", "і", "о"};
    private static final String[] FEMININE_PLURAL = {"и", "", "ам", "и", "ами", "ах", "и"};
    private static final String[] MASCULINE_SINGULAR = {"", "у", "ові", "", "ом", "і", "е"};
    private static final String[] MASCULINE_PLURAL = {"и", "ів", "ам", "и", "ами", "ах", "и"};
    private static final String[] NEUTER_SINGULAR = {"о", "а", "у", "о", "ом", "і", "о"};
    private static final String[] NEUTER_PLURAL = {"а", "", "ам", "а", "ами", "ах", "а"};

    // істота (студент) чи неістота (завод): впливає на родовий та знахідний відмінки
    private boolean animate = false;

    public Noun() {
        super(PartOfSpeech.NOUN);
        gender = Gender.MASCULINE;
        grammaticalCase = GrammaticalCase.NOMINATIVE;
    }

    /** Іменник у називному відмінку однини, напр. new Noun("хат", "а", Gender.FEMININE). */
    public Noun(String root, String ending, Gender gender) {
        this();
        addRoot(root);
        setEnding(ending);
        this.gender = gender;
    }

    @Override
    public void changeForm(GrammaticalNumber newNumber, GrammaticalCase newCase) {
        if (newCase == GrammaticalCase.NONE) {
            throw new IllegalArgumentException("Case is required for a noun");
        }
        String[] table = endingsTable(newNumber);
        String newEnding = table[newCase.ordinal()];

        if (animate) {
            boolean masculineSingular = gender == Gender.MASCULINE && newNumber == GrammaticalNumber.SINGULAR;
            if (masculineSingular && (newCase == GrammaticalCase.GENITIVE || newCase == GrammaticalCase.ACCUSATIVE)) {
                // студента, бачу студента
                newEnding = "а";
            } else if (newNumber == GrammaticalNumber.PLURAL && newCase == GrammaticalCase.ACCUSATIVE) {
                // знахідний множини істот збігається з родовим: бачу студентів
                newEnding = table[GrammaticalCase.GENITIVE.ordinal()];
            }
        }

        setEnding(newEnding);
        number = newNumber;
        grammaticalCase = newCase;
    }

    private String[] endingsTable(GrammaticalNumber newNumber) {
        boolean singular = newNumber == GrammaticalNumber.SINGULAR;
        if (gender == Gender.FEMININE) {
            return singular ? FEMININE_SINGULAR : FEMININE_PLURAL;
        }
        if (gender == Gender.NEUTER) {
            return singular ? NEUTER_SINGULAR : NEUTER_PLURAL;
        }
        if (gender == Gender.MASCULINE) {
            return singular ? MASCULINE_SINGULAR : MASCULINE_PLURAL;
        }
        throw new IllegalStateException("Gender is required for a noun");
    }

    @Override
    public void readFromConsole(Scanner scanner) {
        super.readFromConsole(scanner);
        System.out.print("Істота? (т/н) [н]: ");
        animate = scanner.nextLine().trim().equalsIgnoreCase("т");
    }

    @Override
    protected String getExtraField() {
        return animate ? "animate" : "";
    }

    @Override
    protected void setExtraField(String value) {
        animate = value.equals("animate");
    }

    public boolean isAnimate() {
        return animate;
    }

    public void setAnimate(boolean animate) {
        this.animate = animate;
    }
}
