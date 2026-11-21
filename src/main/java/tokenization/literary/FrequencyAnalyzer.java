package tokenization.literary;

import tokenization.core.Token;
import tokenization.normalization.StopWords;
import tokenization.stats.Frequencies;

/**
 * Частотний аналіз (README_LITCRITIC §9): частоти словоформ, лем
 * та окремо змістових слів (без stop words).
 */
public class FrequencyAnalyzer implements TextAnalyzer {

    private final StopWords stopWords;

    public FrequencyAnalyzer() {
        this(StopWords.loadDefault());
    }

    public FrequencyAnalyzer(StopWords stopWords) {
        this.stopWords = stopWords;
    }

    @Override
    public void analyze(AnalyzedText text, StyleProfile profile) {
        for (Token word : text.getWords()) {
            String value = word.getNormalizedText();
            Frequencies.add(profile.wordFrequencies, value);
            Frequencies.add(profile.lemmaFrequencies, word.getLemma());
            if (!stopWords.isStopWord(value)) {
                Frequencies.add(profile.contentWordFrequencies, value);
            }
        }
    }
}
