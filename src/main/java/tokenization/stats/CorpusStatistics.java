package tokenization.stats;

import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Статистика набору документів (README §25).
 */
public class CorpusStatistics {

    int documents;
    long characters;
    int tokens;
    int uniqueTokens;
    double averageTokenLength;
    int vocabularySize;
    int unknownTokens;
    final Map<String, Integer> tokenFrequencies = new HashMap<>();
    final Map<TokenType, Integer> typeFrequencies = new EnumMap<>(TokenType.class);

    public int getDocuments() {
        return documents;
    }

    public long getCharacters() {
        return characters;
    }

    public int getTokens() {
        return tokens;
    }

    public int getUniqueTokens() {
        return uniqueTokens;
    }

    public double getAverageTokenLength() {
        return averageTokenLength;
    }

    public int getVocabularySize() {
        return vocabularySize;
    }

    public int getUnknownTokens() {
        return unknownTokens;
    }

    public Map<String, Integer> getTokenFrequencies() {
        return tokenFrequencies;
    }

    public Map<TokenType, Integer> getTypeFrequencies() {
        return typeFrequencies;
    }

    /** n найчастіших токенів, від найчастішого до найрідшого. */
    public List<Map.Entry<String, Integer>> topTokens(int n) {
        List<Map.Entry<String, Integer>> entries = new ArrayList<>(tokenFrequencies.entrySet());
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

    public String format(int topCount) {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("%-22s %10d%n", "Documents:", documents));
        sb.append(String.format("%-22s %10d%n", "Characters:", characters));
        sb.append(String.format("%-22s %10d%n", "Tokens:", tokens));
        sb.append(String.format("%-22s %10d%n", "Unique tokens:", uniqueTokens));
        sb.append(String.format("%-22s %10.2f%n", "Average token length:", averageTokenLength));
        sb.append(String.format("%-22s %10d%n", "Vocabulary size:", vocabularySize));
        sb.append(String.format("%-22s %10d%n", "Unknown tokens:", unknownTokens));

        sb.append(String.format("%nTop %d tokens:%n", topCount));
        for (Map.Entry<String, Integer> entry : topTokens(topCount)) {
            sb.append(String.format("  %-20s %8d%n", entry.getKey(), entry.getValue()));
        }

        sb.append(String.format("%nToken types:%n"));
        for (Map.Entry<TokenType, Integer> entry : typeFrequencies.entrySet()) {
            sb.append(String.format("  %-20s %8d%n", entry.getKey(), entry.getValue()));
        }
        return sb.toString();
    }
}
