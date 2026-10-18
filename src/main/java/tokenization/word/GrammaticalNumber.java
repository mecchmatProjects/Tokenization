package tokenization.word;

/**
 * Число.
 */
public enum GrammaticalNumber {
    SINGULAR("однина"),
    PLURAL("множина");

    private final String displayName;

    GrammaticalNumber(String displayName) {
        this.displayName = displayName;
    }

    public String getDisplayName() {
        return displayName;
    }

    @Override
    public String toString() {
        return displayName;
    }
}
