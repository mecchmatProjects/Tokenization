package tokenization.word;

/**
 * Частина мови (тип слова).
 */
public enum PartOfSpeech {
    NOUN("іменник"),
    VERB("дієслово"),
    ADJECTIVE("прикметник"),
    NUMERAL("числівник"),
    PRONOUN("займенник"),
    ADVERB("прислівник"),
    OTHER("інше");

    private final String displayName;

    PartOfSpeech(String displayName) {
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
