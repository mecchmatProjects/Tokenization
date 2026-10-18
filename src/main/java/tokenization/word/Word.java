package tokenization.word;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * Клас СЛОВО (Syllabls.md).
 *
 * Слово зберігається як послідовність частин: префікси (0-3), корені (1-2),
 * суфікси (0-3) та закінчення (0-1; порожній рядок означає нульове закінчення).
 * Текст слова — це просто всі частини, записані підряд.
 */
public class Word {

    public static final int MAX_PREFIXES = 3;
    public static final int MAX_ROOTS = 2;
    public static final int MAX_SUFFIXES = 3;

    protected PartOfSpeech type;
    protected Gender gender = Gender.NONE;
    protected GrammaticalNumber number = GrammaticalNumber.SINGULAR;
    protected Person person = Person.NONE;
    protected GrammaticalCase grammaticalCase = GrammaticalCase.NONE;

    protected final List<String> prefixes = new ArrayList<>();
    protected final List<String> roots = new ArrayList<>();
    protected final List<String> suffixes = new ArrayList<>();
    protected String ending = "";

    public Word() {
        this(PartOfSpeech.OTHER);
    }

    public Word(PartOfSpeech type) {
        this.type = type;
    }

    /** Створює об'єкт потрібного класу-наслідника за типом слова. */
    public static Word create(PartOfSpeech type) {
        switch (type) {
            case NOUN:
                return new Noun();
            case VERB:
                return new Verb();
            case ADJECTIVE:
                return new Adjective();
            case NUMERAL:
                return new Numeral();
            default:
                return new Word(type);
        }
    }

    // ---------- Введення слова по частинах ----------

    public void addPrefix(String prefix) {
        if (prefixes.size() >= MAX_PREFIXES) {
            throw new IllegalStateException("A word can have at most " + MAX_PREFIXES + " prefixes");
        }
        prefixes.add(prefix.toLowerCase());
    }

    public void addRoot(String root) {
        if (roots.size() >= MAX_ROOTS) {
            throw new IllegalStateException("A word can have at most " + MAX_ROOTS + " roots");
        }
        if (root.isEmpty()) {
            throw new IllegalArgumentException("Root cannot be empty");
        }
        roots.add(root.toLowerCase());
    }

    public void addSuffix(String suffix) {
        if (suffixes.size() >= MAX_SUFFIXES) {
            throw new IllegalStateException("A word can have at most " + MAX_SUFFIXES + " suffixes");
        }
        suffixes.add(suffix.toLowerCase());
    }

    public void setEnding(String ending) {
        this.ending = ending == null ? "" : ending.toLowerCase();
    }

    public void clearParts() {
        prefixes.clear();
        roots.clear();
        suffixes.clear();
        ending = "";
    }

    // ---------- Виведення частин слова ----------

    public List<String> getPrefixes() {
        return new ArrayList<>(prefixes);
    }

    public List<String> getRoots() {
        return new ArrayList<>(roots);
    }

    public List<String> getSuffixes() {
        return new ArrayList<>(suffixes);
    }

    public String getEnding() {
        return ending;
    }

    /** Повний текст слова: префікси + корені + суфікси + закінчення. */
    public String getText() {
        StringBuilder sb = new StringBuilder();
        for (String prefix : prefixes) {
            sb.append(prefix);
        }
        for (String root : roots) {
            sb.append(root);
        }
        for (String suffix : suffixes) {
            sb.append(suffix);
        }
        sb.append(ending);
        return sb.toString();
    }

    /** Наприклад: "префікс: при | корінь: ліс | суфікс: ок | закінчення: ∅". */
    public String partsToString() {
        StringBuilder sb = new StringBuilder();
        for (String prefix : prefixes) {
            sb.append("префікс: ").append(prefix).append(" | ");
        }
        for (String root : roots) {
            sb.append("корінь: ").append(root).append(" | ");
        }
        for (String suffix : suffixes) {
            sb.append("суфікс: ").append(suffix).append(" | ");
        }
        sb.append("закінчення: ").append(ending.isEmpty() ? "∅" : ending);
        return sb.toString();
    }

    // ---------- Склади ----------

    /** Розбиття слова на склади за правилами переносу (див. Hyphenator). */
    public List<String> splitIntoSyllables() {
        return new Hyphenator().split(getText());
    }

    /** Склади через дефіс, напр. "при-лі-сок". */
    public String syllablesToString() {
        return String.join("-", splitIntoSyllables()).replace("--", "-");
    }

    /** Злиття складів у слово. */
    public static String mergeSyllables(List<String> syllables) {
        StringBuilder sb = new StringBuilder();
        for (String syllable : syllables) {
            sb.append(syllable);
        }
        return sb.toString();
    }

    // ---------- Властивості ----------

    public void setProperties(Gender gender, GrammaticalNumber number, GrammaticalCase grammaticalCase) {
        this.gender = gender;
        this.number = number;
        this.grammaticalCase = grammaticalCase;
    }

    /**
     * Переводить слово в інше число та відмінок, замінюючи закінчення.
     * Базовий клас не знає правил словозміни, тому це вміють лише наслідники.
     */
    public void changeForm(GrammaticalNumber newNumber, GrammaticalCase newCase) {
        // TODO: таблиці закінчень у наслідниках (Noun, Adjective, Numeral)
        throw new UnsupportedOperationException(type.getDisplayName() + " cannot change its form");
    }

    public PartOfSpeech getType() {
        return type;
    }

    public void setType(PartOfSpeech type) {
        this.type = type;
    }

    public Gender getGender() {
        return gender;
    }

    public void setGender(Gender gender) {
        this.gender = gender;
    }

    public GrammaticalNumber getNumber() {
        return number;
    }

    public void setNumber(GrammaticalNumber number) {
        this.number = number;
    }

    public Person getPerson() {
        return person;
    }

    public void setPerson(Person person) {
        this.person = person;
    }

    public GrammaticalCase getGrammaticalCase() {
        return grammaticalCase;
    }

    public void setGrammaticalCase(GrammaticalCase grammaticalCase) {
        this.grammaticalCase = grammaticalCase;
    }

    // ---------- Введення / виведення в консоль ----------

    public void printToConsole() {
        System.out.println("Слово:      " + getText());
        System.out.println("Тип:        " + type);
        System.out.println("Рід:        " + gender);
        System.out.println("Число:      " + number);
        System.out.println("Особа:      " + person);
        System.out.println("Відмінок:   " + grammaticalCase);
        System.out.println("Частини:    " + partsToString());
    }

    /** Зчитує частини слова та його властивості з консолі. */
    public void readFromConsole(Scanner scanner) {
        clearParts();
        for (String prefix : readParts(scanner, "Префікси (до 3, через кому; Enter — немає)")) {
            addPrefix(prefix);
        }
        List<String> rootList = readParts(scanner, "Корені (1-2, через кому)");
        while (rootList.isEmpty()) {
            rootList = readParts(scanner, "Потрібен хоча б один корінь");
        }
        for (String root : rootList) {
            addRoot(root);
        }
        for (String suffix : readParts(scanner, "Суфікси (до 3, через кому; Enter — немає)")) {
            addSuffix(suffix);
        }
        System.out.print("Закінчення (Enter — нульове): ");
        setEnding(scanner.nextLine().trim());

        gender = Gender.values()[chooseIndex(scanner, "Рід", Gender.values(), gender.ordinal())];
        number = GrammaticalNumber.values()[chooseIndex(scanner, "Число", GrammaticalNumber.values(), number.ordinal())];
        person = Person.values()[chooseIndex(scanner, "Особа", Person.values(), person.ordinal())];
        grammaticalCase = GrammaticalCase.values()[
                chooseIndex(scanner, "Відмінок", GrammaticalCase.values(), grammaticalCase.ordinal())];
    }

    private static List<String> readParts(Scanner scanner, String prompt) {
        System.out.print(prompt + ": ");
        List<String> parts = new ArrayList<>();
        for (String part : scanner.nextLine().split(",")) {
            if (!part.trim().isEmpty()) {
                parts.add(part.trim());
            }
        }
        return parts;
    }

    /** Показує нумерований список значень і повертає індекс вибраного. */
    private static int chooseIndex(Scanner scanner, String title, Object[] values, int current) {
        StringBuilder sb = new StringBuilder(title + ": ");
        for (int i = 0; i < values.length; i++) {
            sb.append(i + 1).append(") ").append(values[i]).append("  ");
        }
        System.out.println(sb);
        System.out.print("Ваш вибір [" + (current + 1) + "]: ");
        String line = scanner.nextLine().trim();
        try {
            int choice = Integer.parseInt(line) - 1;
            if (choice >= 0 && choice < values.length) {
                return choice;
            }
        } catch (NumberFormatException e) {
            // залишаємо поточне значення
        }
        return current;
    }

    // ---------- Введення / виведення у файл ----------

    /**
     * Один рядок файлу:
     * ТИП;префікси;корені;суфікси;закінчення;РІД;ЧИСЛО;ОСОБА;ВІДМІНОК
     * частини одного виду записуються через кому, напр. NOUN;при;ліс;ок;;MASCULINE;SINGULAR;NONE;NOMINATIVE
     */
    public String toFileLine() {
        return type.name() + ";" + String.join(",", prefixes) + ";" + String.join(",", roots) + ";"
                + String.join(",", suffixes) + ";" + ending + ";" + gender.name() + ";" + number.name() + ";"
                + person.name() + ";" + grammaticalCase.name();
    }

    public static Word fromFileLine(String line) {
        // -1, щоб порожні поля в кінці рядка не відкидались
        String[] fields = line.split(";", -1);
        if (fields.length != 9) {
            throw new IllegalArgumentException("Expected 9 fields, got " + fields.length + ": " + line);
        }
        Word word = create(PartOfSpeech.valueOf(fields[0]));
        for (String prefix : splitList(fields[1])) {
            word.addPrefix(prefix);
        }
        for (String root : splitList(fields[2])) {
            word.addRoot(root);
        }
        for (String suffix : splitList(fields[3])) {
            word.addSuffix(suffix);
        }
        word.setEnding(fields[4]);
        word.gender = Gender.valueOf(fields[5]);
        word.number = GrammaticalNumber.valueOf(fields[6]);
        word.person = Person.valueOf(fields[7]);
        word.grammaticalCase = GrammaticalCase.valueOf(fields[8]);
        return word;
    }

    private static List<String> splitList(String field) {
        List<String> result = new ArrayList<>();
        for (String part : field.split(",")) {
            if (!part.isEmpty()) {
                result.add(part);
            }
        }
        return result;
    }

    public static void saveToFile(List<Word> words, Path file) throws IOException {
        try (BufferedWriter writer = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            for (Word word : words) {
                writer.write(word.toFileLine());
                writer.newLine();
            }
        }
    }

    public static List<Word> loadFromFile(Path file) throws IOException {
        List<Word> words = new ArrayList<>();
        for (String line : Files.readAllLines(file, StandardCharsets.UTF_8)) {
            if (!line.isBlank() && !line.startsWith("#")) {
                words.add(fromFileLine(line.trim()));
            }
        }
        return words;
    }

    @Override
    public String toString() {
        return getText();
    }
}
