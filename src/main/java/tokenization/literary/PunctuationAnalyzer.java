package tokenization.literary;

import tokenization.core.Token;
import tokenization.core.TokenType;
import tokenization.stats.Frequencies;

/**
 * Аналіз пунктуації (README_LITCRITIC §13): частота кожного знака
 * та середня кількість ком і всіх розділових знаків на речення.
 */
public class PunctuationAnalyzer implements TextAnalyzer {

    @Override
    public void analyze(AnalyzedText text, StyleProfile profile) {
        int total = 0;
        for (Token token : text.getTokens()) {
            if (token.getType() == TokenType.PUNCTUATION) {
                // "…" одним символом і "..." — це той самий знак
                String mark = token.getText().equals("…") ? "..." : token.getText();
                Frequencies.add(profile.punctuationFrequencies, mark);
                total++;
            }
        }

        int sentences = text.getSentences().size();
        if (sentences > 0) {
            profile.commasPerSentence = (double) profile.getPunctuationCount(",") / sentences;
            profile.punctuationPerSentence = (double) total / sentences;
        }
    }
}
