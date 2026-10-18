package tokenization.ui;

import java.util.Scanner;

/**
 * Зчитування відповідей користувача з консолі зі значеннями за замовчуванням.
 */
public class ConsoleInput {

    private final Scanner scanner;

    public ConsoleInput(Scanner scanner) {
        this.scanner = scanner;
    }

    /** Рядок від користувача; якщо введено порожній рядок — значення за замовчуванням. */
    public String readLine(String prompt, String defaultValue) {
        if (defaultValue != null && !defaultValue.isEmpty()) {
            System.out.print(prompt + " [" + defaultValue + "]: ");
        } else {
            System.out.print(prompt + ": ");
        }
        if (!scanner.hasNextLine()) {
            return defaultValue;
        }
        String line = scanner.nextLine().trim();
        return line.isEmpty() ? defaultValue : line;
    }

    public String readLine(String prompt) {
        return readLine(prompt, "");
    }

    public int readInt(String prompt, int defaultValue) {
        while (true) {
            String line = readLine(prompt, String.valueOf(defaultValue));
            try {
                return Integer.parseInt(line);
            } catch (NumberFormatException e) {
                System.out.println("Please enter a number.");
            }
        }
    }

    public boolean readYesNo(String prompt, boolean defaultValue) {
        String line = readLine(prompt + " (y/n)", defaultValue ? "y" : "n");
        return line.equalsIgnoreCase("y") || line.equalsIgnoreCase("yes");
    }

    /** Номер пункту меню; -1, якщо введено не число. */
    public int readChoice() {
        System.out.print("> ");
        // кінець вводу (наприклад, Ctrl+D) — вихід з меню
        if (!scanner.hasNextLine()) {
            return 0;
        }
        try {
            return Integer.parseInt(scanner.nextLine().trim());
        } catch (NumberFormatException e) {
            return -1;
        }
    }
}
