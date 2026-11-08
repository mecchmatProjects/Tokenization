package tokenization.stats;

import tokenization.core.Token;
import tokenization.pipeline.ProcessedDocument;
import tokenization.vocabulary.Vocabulary;

import java.util.List;

/**
 * Обчислює статистику оброблених документів (README §25).
 */
public class StatisticsAnalyzer {

    /**
     * @param vocabulary словник; може бути null, якщо його ще не побудовано
     */
    public CorpusStatistics analyze(List<ProcessedDocument> documents, Vocabulary vocabulary) {
        CorpusStatistics stats = new CorpusStatistics();
        stats.documents = documents.size();

        long totalLength = 0;
        for (ProcessedDocument document : documents) {
            stats.characters += document.getDocument().getContent().length();

            for (Token token : document.getNormalizedTokens()) {
                String text = token.getNormalizedText();
                stats.tokens++;
                totalLength += text.length();
                stats.tokenFrequencies.put(text, stats.tokenFrequencies.getOrDefault(text, 0) + 1);
                stats.typeFrequencies.put(token.getType(), stats.typeFrequencies.getOrDefault(token.getType(), 0) + 1);

                if (vocabulary != null && vocabulary.getId(text) == Vocabulary.UNK_ID) {
                    stats.unknownTokens++;
                }
            }
        }

        stats.uniqueTokens = stats.tokenFrequencies.size();
        stats.averageTokenLength = stats.tokens > 0 ? (double) totalLength / stats.tokens : 0;
        stats.vocabularySize = vocabulary != null ? vocabulary.size() : 0;
        return stats;
    }
}
