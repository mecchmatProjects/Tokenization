package tokenization.normalization;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.Set;

/**
 * Набір службових слів (README §14). Списки зберігаються в resources/stopwords.
 */
public class StopWords {

    private final Set<String> words;

    public StopWords(Set<String> words) {
        this.words = new HashSet<>();
        for (String word : words) {
            this.words.add(word.toLowerCase());
        }
    }

    /** Англійські та українські stop words з ресурсів програми. */
    public static StopWords loadDefault() {
        Set<String> words = new HashSet<>();
        words.addAll(loadResource("/stopwords/en.txt"));
        words.addAll(loadResource("/stopwords/uk.txt"));
        return new StopWords(words);
    }

    public boolean isStopWord(String word) {
        return words.contains(word.toLowerCase());
    }

    public int size() {
        return words.size();
    }

    // Один рядок — одне слово, рядки з # — коментарі
    private static Set<String> loadResource(String name) {
        Set<String> result = new HashSet<>();
        InputStream stream = StopWords.class.getResourceAsStream(name);
        if (stream == null) {
            throw new IllegalStateException("Resource not found: " + name);
        }
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (!line.isEmpty() && !line.startsWith("#")) {
                    result.add(line);
                }
            }
        } catch (IOException e) {
            throw new IllegalStateException("Cannot read " + name, e);
        }
        return result;
    }
}
