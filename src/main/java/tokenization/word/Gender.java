package tokenization.word;

/**
 * Рід.
 */
public enum Gender {
    MASCULINE("чоловічий"),
    FEMININE("жіночий"),
    NEUTER("середній"),
    NONE("—");

    private final String displayName;

    Gender(String displayName) {
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
