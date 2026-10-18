package tokenization.ui;

import tokenization.word.Adjective;
import tokenization.word.Gender;
import tokenization.word.GrammaticalCase;
import tokenization.word.GrammaticalNumber;
import tokenization.word.Hyphenator;
import tokenization.word.Numeral;
import tokenization.word.PartOfSpeech;
import tokenization.word.Person;
import tokenization.word.Verb;
import tokenization.word.Word;

import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * Консольне меню для роботи з класом Word (Syllabls.md).
 */
public class WordMenu {

    // відмінки без NONE — для вибору в меню
    private static final GrammaticalCase[] CASES = {
            GrammaticalCase.NOMINATIVE, GrammaticalCase.GENITIVE, GrammaticalCase.DATIVE,
            GrammaticalCase.ACCUSATIVE, GrammaticalCase.INSTRUMENTAL, GrammaticalCase.LOCATIVE,
            GrammaticalCase.VOCATIVE
    };
    private static final Gender[] GENDERS = {Gender.MASCULINE, Gender.FEMININE, Gender.NEUTER};
    private static final Person[] PERSONS = {Person.FIRST, Person.SECOND, Person.THIRD};

    private final ConsoleInput input;
    private final Scanner scanner;
    private List<Word> words = new ArrayList<>();
    private Word current;

    public WordMenu(ConsoleInput input) {
        this.input = input;
        this.scanner = input.getScanner();
    }

    public void run() {
        while (true) {
            System.out.println();
            System.out.println("=== Слово та склади ===");
            System.out.println("Поточне слово: " + (current == null ? "—" : current.getText()));
            System.out.println("1. Ввести нове слово по частинах");
            System.out.println("2. Вибрати слово зі списку");
            System.out.println("3. Показати поточне слово");
            System.out.println("4. Поділити поточне слово на склади");
            System.out.println("5. Змінити форму поточного слова");
            System.out.println("6. Поділити на склади довільне слово");
            System.out.println("7. Зберегти слова у файл");
            System.out.println("8. Завантажити слова з файлу");
            System.out.println("0. Назад");

            int choice = input.readChoice();
            switch (choice) {
                case 1 -> enterWord();
                case 2 -> selectWord();
                case 3 -> showCurrent();
                case 4 -> splitCurrent();
                case 5 -> changeForm();
                case 6 -> splitAnyWord();
                case 7 -> save();
                case 8 -> load();
                case 0 -> {
                    return;
                }
                default -> System.out.println("Невідомий пункт.");
            }
        }
    }

    private void enterWord() {
        PartOfSpeech[] types = PartOfSpeech.values();
        int index = Word.chooseIndex(scanner, "Тип слова", types, 0);
        Word word = Word.create(types[index]);
        word.readFromConsole(scanner);
        words.add(word);
        current = word;
        System.out.println("Додано: " + word.getText());
    }

    private void selectWord() {
        if (words.isEmpty()) {
            System.out.println("Список слів порожній.");
            return;
        }
        for (int i = 0; i < words.size(); i++) {
            Word word = words.get(i);
            System.out.println((i + 1) + ". " + word.getText() + " (" + word.getType() + ")");
        }
        int index = input.readInt("Номер слова", 1) - 1;
        if (index >= 0 && index < words.size()) {
            current = words.get(index);
        }
    }

    private boolean hasCurrent() {
        if (current == null) {
            System.out.println("Спочатку введіть або виберіть слово.");
            return false;
        }
        return true;
    }

    private void showCurrent() {
        if (hasCurrent()) {
            current.printToConsole();
            System.out.println("Склади:     " + current.syllablesToString());
        }
    }

    private void splitCurrent() {
        if (!hasCurrent()) {
            return;
        }
        List<String> syllables = current.splitIntoSyllables();
        System.out.println("Склади: " + syllables);
        System.out.println("Злиття складів: " + Word.mergeSyllables(syllables));
    }

    private void changeForm() {
        if (!hasCurrent()) {
            return;
        }
        try {
            if (current instanceof Verb) {
                GrammaticalNumber number = chooseNumber();
                Person person = PERSONS[Word.chooseIndex(scanner, "Особа", PERSONS, 2)];
                ((Verb) current).changeForm(number, person);
            } else if (current instanceof Adjective) {
                Gender gender = GENDERS[Word.chooseIndex(scanner, "Рід", GENDERS, 0)];
                ((Adjective) current).changeForm(gender, chooseNumber(), chooseCase());
            } else if (current instanceof Numeral) {
                Gender gender = GENDERS[Word.chooseIndex(scanner, "Рід", GENDERS, 0)];
                ((Numeral) current).changeForm(gender, chooseNumber(), chooseCase());
            } else {
                current.changeForm(chooseNumber(), chooseCase());
            }
            System.out.println("Нова форма: " + current.getText());
        } catch (UnsupportedOperationException | IllegalArgumentException | IllegalStateException e) {
            System.out.println("Неможливо змінити форму: " + e.getMessage());
        }
    }

    private GrammaticalNumber chooseNumber() {
        GrammaticalNumber[] numbers = GrammaticalNumber.values();
        return numbers[Word.chooseIndex(scanner, "Число", numbers, 0)];
    }

    private GrammaticalCase chooseCase() {
        return CASES[Word.chooseIndex(scanner, "Відмінок", CASES, 0)];
    }

    private void splitAnyWord() {
        String text = input.readLine("Слово");
        if (text == null || text.isEmpty()) {
            return;
        }
        List<String> parts = new Hyphenator().split(text);
        System.out.println("Частини для переносу: " + String.join(" | ", parts));
        System.out.println("Кількість складів (голосних): " + Hyphenator.countSyllables(text));
    }

    private void save() {
        String path = input.readLine("Файл", "words/words.txt");
        try {
            Word.saveToFile(words, Path.of(path));
            System.out.println("Збережено слів: " + words.size());
        } catch (IOException e) {
            System.out.println("Помилка запису: " + e.getMessage());
        }
    }

    private void load() {
        String path = input.readLine("Файл", "words/words.txt");
        try {
            words = Word.loadFromFile(Path.of(path));
            current = words.isEmpty() ? null : words.get(0);
            System.out.println("Завантажено слів: " + words.size());
        } catch (IOException | IllegalArgumentException e) {
            System.out.println("Помилка читання: " + e.getMessage());
        }
    }
}
