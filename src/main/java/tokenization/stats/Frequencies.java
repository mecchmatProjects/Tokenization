package tokenization.stats;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Допоміжні методи для таблиць частот (слово → кількість).
 */
public class Frequencies {

    private Frequencies() {
    }

    public static void add(Map<String, Integer> frequencies, String key) {
        frequencies.put(key, frequencies.getOrDefault(key, 0) + 1);
    }

    /** n найчастіших записів; при однаковій частоті — за алфавітом. */
    public static List<Map.Entry<String, Integer>> top(Map<String, Integer> frequencies, int n) {
        List<Map.Entry<String, Integer>> entries = new ArrayList<>(frequencies.entrySet());
        entries.sort((a, b) -> {
            if (!a.getValue().equals(b.getValue())) {
                return b.getValue() - a.getValue();
            }
            return a.getKey().compareTo(b.getKey());
        });
        if (entries.size() > n) {
            return new ArrayList<>(entries.subList(0, n));
        }
        return entries;
    }
}
