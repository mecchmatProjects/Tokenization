package tokenization.word;

import java.util.ArrayList;
import java.util.List;

/**
 * Поділ українського слова на частини для переносу (Syllabls.md).
 *
 * Слово спочатку розбивається на «одиниці»: зазвичай одна літера, але
 * - "й" об'єднується з попередньою голосною ("ай", "ій");
 * - "ь" та апостроф об'єднуються з попередньою приголосною ("ль", "м'").
 * Одиниця вважається голосною, якщо починається з голосної літери.
 *
 * Місце поділу шукається в такому порядку:
 * 0. Приставка зі списку на початку слова, якщо після неї йде приголосна,
 *    а в решті слова є голосна (пере-бігти, при-лісок).
 * 1. Дві голосні підряд: ділимо між ними, якщо перед першою є приголосна,
 *    а після другої — хоча б одна літера (по-ет).
 * 2. Дві приголосні підряд: ділимо між ними, якщо перед першою є голосна,
 *    а після другої є голосна (сес-тра, мор-ква).
 * 3. Інакше: перша частина довша за одну літеру й закінчується голосною,
 *    друга містить голосну (ма-ма, лій-ка).
 * Після поділу обидві частини обробляються так само, доки ділити вже нема де.
 */
public class Hyphenator {

    private static final String VOWELS = "аеєиіїоуюя";

    // приставки, що містять голосні; довші йдуть першими ("перед" раніше за "пере")
    private static final String[] PREFIXES = {
            "перед", "понад", "через", "пере", "роз", "без", "від", "під", "над",
            "при", "пре", "про", "най", "за", "на", "по", "до", "об"
    };

    /** Частини слова для переносу. Дефіс залишається в кінці частини перед ним (будь- який). */
    public List<String> split(String word) {
        List<String> result = new ArrayList<>();
        if (word == null || word.isEmpty()) {
            return result;
        }

        String[] pieces = word.split("-");
        for (int i = 0; i < pieces.length; i++) {
            List<String> parts = splitSimpleWord(pieces[i]);
            if (i < pieces.length - 1 && !parts.isEmpty()) {
                int last = parts.size() - 1;
                parts.set(last, parts.get(last) + "-");
            }
            result.addAll(parts);
        }
        return result;
    }

    /** Кількість складів в українському слові дорівнює кількості голосних. */
    public static int countSyllables(String word) {
        int count = 0;
        for (int i = 0; i < word.length(); i++) {
            if (isVowel(word.charAt(i))) {
                count++;
            }
        }
        return count;
    }

    private List<String> splitSimpleWord(String word) {
        List<String> result = new ArrayList<>();
        if (word.isEmpty()) {
            return result;
        }
        List<String> units = toUnits(word);
        splitSegment(units, 0, units.size(), result);
        return result;
    }

    /** Рекурсивно ділить одиниці з індексами [from, to) і додає частини до result. */
    private void splitSegment(List<String> units, int from, int to, List<String> result) {
        int position = findBreak(units, from, to);
        if (position < 0) {
            result.add(join(units, from, to));
            return;
        }
        splitSegment(units, from, position, result);
        splitSegment(units, position, to, result);
    }

    /** Індекс першої одиниці другої частини, або -1, якщо поділити не можна. */
    private int findBreak(List<String> units, int from, int to) {
        if (from == 0) {
            int prefixEnd = findPrefix(units, to);
            if (prefixEnd > 0) {
                return prefixEnd;
            }
        }

        // правило 1: голосна | голосна
        for (int p = from + 2; p + 1 < to; p++) {
            if (!isVowelUnit(units.get(p - 2)) && isVowelUnit(units.get(p - 1)) && isVowelUnit(units.get(p))) {
                return p;
            }
        }

        // правило 2: приголосна | приголосна
        for (int p = from + 2; p < to; p++) {
            if (isVowelUnit(units.get(p - 2)) && !isVowelUnit(units.get(p - 1)) && !isVowelUnit(units.get(p))
                    && hasVowel(units, p + 1, to)) {
                return p;
            }
        }

        // правило 3: перша частина > 1 літери і закінчується голосною, у другій є голосна
        for (int p = from + 1; p < to; p++) {
            if (letterCount(units, from, p) > 1 && isVowelUnit(units.get(p - 1)) && hasVowel(units, p, to)) {
                return p;
            }
        }
        return -1;
    }

    /** Кількість одиниць, які займає приставка на початку слова, або -1. */
    private int findPrefix(List<String> units, int to) {
        String word = join(units, 0, to).toLowerCase();
        for (String prefix : PREFIXES) {
            if (!word.startsWith(prefix)) {
                continue;
            }
            // приставка має закінчуватися на межі одиниць ("на" не можна виділити з "найкращий")
            int length = 0;
            int unitCount = 0;
            while (unitCount < to && length < prefix.length()) {
                length += units.get(unitCount).length();
                unitCount++;
            }
            if (length != prefix.length() || unitCount >= to) {
                continue;
            }
            if (!isVowelUnit(units.get(unitCount)) && hasVowel(units, unitCount, to)) {
                return unitCount;
            }
        }
        return -1;
    }

    private List<String> toUnits(String word) {
        List<String> units = new ArrayList<>();
        for (int i = 0; i < word.length(); i++) {
            char c = word.charAt(i);
            char lower = Character.toLowerCase(c);
            boolean attachToPrevious = false;
            if (!units.isEmpty()) {
                String previous = units.get(units.size() - 1);
                if (lower == 'й' && previous.length() == 1 && isVowel(previous.charAt(0))) {
                    attachToPrevious = true;
                } else if ((lower == 'ь' || isApostrophe(c)) && !isVowelUnit(previous)) {
                    attachToPrevious = true;
                }
            }
            if (attachToPrevious) {
                units.set(units.size() - 1, units.get(units.size() - 1) + c);
            } else {
                units.add(String.valueOf(c));
            }
        }
        return units;
    }

    private boolean hasVowel(List<String> units, int from, int to) {
        for (int i = from; i < to; i++) {
            if (isVowelUnit(units.get(i))) {
                return true;
            }
        }
        return false;
    }

    private int letterCount(List<String> units, int from, int to) {
        int count = 0;
        for (int i = from; i < to; i++) {
            for (char c : units.get(i).toCharArray()) {
                if (Character.isLetter(c)) {
                    count++;
                }
            }
        }
        return count;
    }

    private String join(List<String> units, int from, int to) {
        StringBuilder sb = new StringBuilder();
        for (int i = from; i < to; i++) {
            sb.append(units.get(i));
        }
        return sb.toString();
    }

    private static boolean isVowelUnit(String unit) {
        return isVowel(unit.charAt(0));
    }

    private static boolean isVowel(char c) {
        return VOWELS.indexOf(Character.toLowerCase(c)) >= 0;
    }

    private static boolean isApostrophe(char c) {
        return c == '\'' || c == '’' || c == 'ʼ';
    }
}
