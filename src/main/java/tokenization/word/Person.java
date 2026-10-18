package tokenization.word;

/**
 * Особа (для дієслів і займенників).
 */
public enum Person {
    FIRST("1-ша"),
    SECOND("2-га"),
    THIRD("3-тя"),
    NONE("—");

    private final String displayName;

    Person(String displayName) {
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
