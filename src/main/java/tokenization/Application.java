package tokenization;

import tokenization.ui.ConsoleInput;
import tokenization.ui.TokenizerMenu;

import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.Scanner;

/**
 * Точка входу в програму.
 */
public class Application {

    public static void main(String[] args) {
        // дробові числа виводимо з крапкою (5.73), як у README, незалежно від мови системи
        Locale.setDefault(Locale.US);
        ConsoleInput input = new ConsoleInput(new Scanner(System.in, StandardCharsets.UTF_8));

        while (true) {
            System.out.println();
            System.out.println("=================================");
            System.out.println(" Text Processing Toolkit");
            System.out.println("=================================");
            System.out.println("1. Text tokenizer (ML dataset)");
            // TODO: 2. Literary text analyzer (README_LITCRITIC.md)
            // TODO: 3. Ukrainian word and syllables (Syllabls.md)
            System.out.println("0. Exit");

            int choice = input.readChoice();
            if (choice == 1) {
                new TokenizerMenu(input).run();
            } else if (choice == 0) {
                return;
            } else {
                System.out.println("Unknown option.");
            }
        }
    }
}
