package tokenization.word;

import java.util.Scanner;

/**
 * ЧИСЛІВНИК.
 *
 * Порядкові числівники твердої групи (перший, другий, п'ятий) відмінюються як прикметники.
 * Кількісні числівники (два, п'ять, сорок) мають неправильне відмінювання і тут не змінюються.
 */
public class Numeral extends Word {

    private boolean ordinal;

    public Numeral() {
        super(PartOfSpeech.NUMERAL);
        gender = Gender.MASCULINE;
        grammaticalCase = GrammaticalCase.NOMINATIVE;
    }

    /** Напр. new Numeral("перш", "ий", true) або new Numeral("п'ять", "", false). */
    public Numeral(String root, String ending, boolean ordinal) {
        this();
        addRoot(root);
        setEnding(ending);
        this.ordinal = ordinal;
    }

    @Override
    public void changeForm(GrammaticalNumber newNumber, GrammaticalCase newCase) {
        changeForm(gender, newNumber, newCase);
    }

    public void changeForm(Gender newGender, GrammaticalNumber newNumber, GrammaticalCase newCase) {
        if (!ordinal) {
            throw new UnsupportedOperationException("Only ordinal numerals can change form");
        }
        setEnding(Adjective.hardEnding(newGender, newNumber, newCase));
        gender = newGender;
        number = newNumber;
        grammaticalCase = newCase;
    }

    @Override
    public void readFromConsole(Scanner scanner) {
        super.readFromConsole(scanner);
        System.out.print("Порядковий числівник? (т/н) [н]: ");
        ordinal = scanner.nextLine().trim().equalsIgnoreCase("т");
    }

    @Override
    protected String getExtraField() {
        return ordinal ? "ordinal" : "";
    }

    @Override
    protected void setExtraField(String value) {
        ordinal = value.equals("ordinal");
    }

    public boolean isOrdinal() {
        return ordinal;
    }

    public void setOrdinal(boolean ordinal) {
        this.ordinal = ordinal;
    }
}
