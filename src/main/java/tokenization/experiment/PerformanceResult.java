package tokenization.experiment;

/**
 * Час виконання етапів pipeline для одного розміру даних (README §32).
 */
public class PerformanceResult {

    int sizeKb;
    int characters;
    int tokens;
    double loadMs;
    double preprocessMs;
    double tokenizeMs;
    double vocabularyMs;
    double encodeMs;
    double exportMs;

    public int getSizeKb() {
        return sizeKb;
    }

    public int getCharacters() {
        return characters;
    }

    public int getTokens() {
        return tokens;
    }

    public double getTotalMs() {
        return loadMs + preprocessMs + tokenizeMs + vocabularyMs + encodeMs + exportMs;
    }
}
