package tokenization.normalization;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

/**
 * Спрощена лематизація (README §15, README_LITCRITIC §6).
 *
 * Повноцінний морфологічний аналізатор виходить за межі курсової, тому використовується
 * комбінація двох підходів:
 * 1. Словник: для поширених слів з чергуванням (будинок/будинку, йти/йшов) лема береться з
 *    файлу resources/lemmas.txt.
 * 2. Відкидання закінчень: для решти слів відкидається найдовше відоме закінчення,
 *    якщо після цього залишається основа щонайменше з MIN_STEM_LENGTH літер.
 *
 * Для слів, яких немає у словнику, результатом є основа (стем), а не справжня лема,
 * але різні форми одного слова зазвичай дають однакову основу.
 */
public class Lemmatizer {

    private static final int MIN_STEM_LENGTH = 3;

    // від довших до коротших, щоб "ами" перевірялось раніше за "и"
    private static final String[] UKRAINIAN_ENDINGS = {
            "ями", "ами", "ові", "еві", "ого", "ому", "ими", "іми", "ють", "ять", "уть",
            "ємо", "имо", "емо", "єте", "ите", "ете",
            "ою", "ею", "єю", "ій", "ий", "ів", "їв", "ах", "ях", "ам", "ям", "ом", "ем",
            "им", "их", "іх", "ти", "ть", "єш", "иш", "еш",
            "а", "я", "у", "ю", "і", "и", "о", "е", "є", "ї"
    };

    private static final String[] ENGLISH_SUFFIXES = {
            "ing", "ion", "ed", "es", "ly", "s"
    };

    private final Map<String, String> dictionary = new HashMap<>();

    public Lemmatizer() {
        loadDictionary("/lemmas.txt");
    }

    public String lemmatize(String word) {
        String lower = word.toLowerCase();
        String lemma = dictionary.get(lower);
        if (lemma != null) {
            return lemma;
        }
        if (isCyrillic(lower)) {
            return stripUkrainianEnding(lower);
        }
        return stripEnglishSuffix(lower);
    }

    private String stripUkrainianEnding(String word) {
        for (String ending : UKRAINIAN_ENDINGS) {
            if (word.endsWith(ending) && word.length() - ending.length() >= MIN_STEM_LENGTH) {
                return word.substring(0, word.length() - ending.length());
            }
        }
        return word;
    }

    private String stripEnglishSuffix(String word) {
        // studies → study
        if (word.endsWith("ies") && word.length() - 3 >= MIN_STEM_LENGTH) {
            return word.substring(0, word.length() - 3) + "y";
        }
        for (String suffix : ENGLISH_SUFFIXES) {
            if (!word.endsWith(suffix) || word.length() - suffix.length() < MIN_STEM_LENGTH) {
                continue;
            }
            // class, this, status — кінцеве "s" тут не є закінченням множини
            if (suffix.equals("s") && (word.endsWith("ss") || word.endsWith("is") || word.endsWith("us"))) {
                continue;
            }
            return word.substring(0, word.length() - suffix.length());
        }
        return word;
    }

    private boolean isCyrillic(String word) {
        for (int i = 0; i < word.length(); i++) {
            if (Character.UnicodeBlock.of(word.charAt(i)) == Character.UnicodeBlock.CYRILLIC) {
                return true;
            }
        }
        return false;
    }

    /**
     * Формат файлу: у кожному рядку спочатку лема, далі її форми через пробіл.
     * Наприклад: "бути був була було були є буде будуть"
     */
    private void loadDictionary(String resource) {
        InputStream stream = Lemmatizer.class.getResourceAsStream(resource);
        if (stream == null) {
            throw new IllegalStateException("Resource not found: " + resource);
        }
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) {
                    continue;
                }
                String[] words = line.split("\\s+");
                String lemma = words[0];
                for (String form : words) {
                    dictionary.put(form, lemma);
                }
            }
        } catch (IOException e) {
            throw new IllegalStateException("Cannot read " + resource, e);
        }
    }

    public int dictionarySize() {
        return dictionary.size();
    }
}
