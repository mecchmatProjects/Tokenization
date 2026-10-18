package tokenization.word;

/**
 * Відмінок. Порядок констант відповідає порядку відмінків у таблицях закінчень.
 */
public enum GrammaticalCase {
    NOMINATIVE("називний"),
    GENITIVE("родовий"),
    DATIVE("давальний"),
    ACCUSATIVE("знахідний"),
    INSTRUMENTAL("орудний"),
    LOCATIVE("місцевий"),
    VOCATIVE("кличний"),
    NONE("—");

    private final String displayName;

    GrammaticalCase(String displayName) {
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
