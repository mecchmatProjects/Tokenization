package tokenization.vocabulary;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Двонапрямлений словник token ↔ ID (README §16-19).
 *
 * Перші чотири ID зарезервовані під спеціальні токени: 0 <PAD>, 1 <UNK>, 2 <START>, 3 <END>.
 * Звичайні токени отримують ID у порядку спадання частоти (при однаковій частоті — за алфавітом).
 */
public class Vocabulary {

    public static final String PAD = "<PAD>";
    public static final String UNK = "<UNK>";
    public static final String START = "<START>";
    public static final String END = "<END>";

    public static final int PAD_ID = 0;
    public static final int UNK_ID = 1;
    public static final int START_ID = 2;
    public static final int END_ID = 3;
    public static final int SPECIAL_TOKEN_COUNT = 4;

    private final Map<String, Integer> tokenToId = new HashMap<>();
    private final List<String> idToToken = new ArrayList<>();
    private final Map<String, Integer> frequencies = new HashMap<>();

    public Vocabulary() {
        add(PAD, 0);
        add(UNK, 0);
        add(START, 0);
        add(END, 0);
    }

    /**
     * Будує словник за набором документів.
     *
     * @param documents    кожен документ — список нормалізованих токенів
     * @param minFrequency токен потрапляє до словника, якщо зустрічається щонайменше стільки разів
     * @param maxSize      максимальна кількість звичайних токенів (без спеціальних); 0 — без обмеження
     */
    public static Vocabulary build(List<List<String>> documents, int minFrequency, int maxSize) {
        Map<String, Integer> counts = countFrequencies(documents);

        List<Map.Entry<String, Integer>> entries = new ArrayList<>();
        for (Map.Entry<String, Integer> entry : counts.entrySet()) {
            if (entry.getValue() >= minFrequency) {
                entries.add(entry);
            }
        }
        entries.sort((a, b) -> {
            if (!a.getValue().equals(b.getValue())) {
                return b.getValue() - a.getValue();
            }
            return a.getKey().compareTo(b.getKey());
        });

        Vocabulary vocabulary = new Vocabulary();
        for (Map.Entry<String, Integer> entry : entries) {
            if (maxSize > 0 && vocabulary.size() - SPECIAL_TOKEN_COUNT >= maxSize) {
                break;
            }
            vocabulary.add(entry.getKey(), entry.getValue());
        }
        return vocabulary;
    }

    /** Кількість появ кожного токена в усіх документах. */
    public static Map<String, Integer> countFrequencies(List<List<String>> documents) {
        Map<String, Integer> counts = new HashMap<>();
        for (List<String> document : documents) {
            for (String token : document) {
                counts.put(token, counts.getOrDefault(token, 0) + 1);
            }
        }
        return counts;
    }

    private void add(String token, int frequency) {
        if (tokenToId.containsKey(token)) {
            return;
        }
        tokenToId.put(token, idToToken.size());
        idToToken.add(token);
        frequencies.put(token, frequency);
    }

    /** ID токена або ID <UNK>, якщо токена немає у словнику. */
    public int getId(String token) {
        Integer id = tokenToId.get(token);
        return id != null ? id : UNK_ID;
    }

    /** Токен за ID або <UNK>, якщо такого ID немає. */
    public String getToken(int id) {
        if (id < 0 || id >= idToToken.size()) {
            return UNK;
        }
        return idToToken.get(id);
    }

    public boolean contains(String token) {
        return tokenToId.containsKey(token);
    }

    public int getFrequency(String token) {
        return frequencies.getOrDefault(token, 0);
    }

    /** Кількість токенів разом зі спеціальними. */
    public int size() {
        return idToToken.size();
    }

    /** Усі токени в порядку їхніх ID. */
    public List<String> getTokens() {
        return new ArrayList<>(idToToken);
    }
}
