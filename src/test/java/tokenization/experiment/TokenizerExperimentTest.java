package tokenization.experiment;

import org.junit.jupiter.api.Test;
import tokenization.core.Document;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class TokenizerExperimentTest {

    private final TokenizerExperiment experiment = new TokenizerExperiment();
    private final List<Document> documents = List.of(
            new Document(1, "a", "Hello, world! Hello again."),
            new Document(2, "b", "The world is big. THE END"));

    @Test
    void countsTokensForReadmeTable() {
        // README §26: Whitespace / Basic / Rule-based
        assertArrayEquals(new int[]{1, 2, 2}, experiment.countTokens("Hello,"));
        assertArrayEquals(new int[]{1, 5, 1}, experiment.countTokens("test@example.com"));
        assertArrayEquals(new int[]{1, 3, 1}, experiment.countTokens("19.99"));
        assertArrayEquals(new int[]{1, 3, 1}, experiment.countTokens("C++"));
    }

    @Test
    void examplesTableContainsAllTexts() {
        String table = experiment.compareOnExamples();
        for (String example : TokenizerExperiment.EXAMPLES) {
            assertTrue(table.contains(example));
        }
    }

    @Test
    void strategyComparisonListsAllStrategies() {
        String table = experiment.compareStrategies(documents);
        assertTrue(table.contains("Whitespace"));
        assertTrue(table.contains("Basic"));
        assertTrue(table.contains("Rule-based"));
    }

    @Test
    void lowercaseReducesVocabulary() {
        // "The" і "THE" у lowercase-режимі стають одним токеном
        int caseSensitive = experiment.vocabularySize(documents, false);
        int lowercase = experiment.vocabularySize(documents, true);
        assertEquals(caseSensitive - 1, lowercase);
        assertTrue(experiment.lowercaseExperiment(documents).contains("case-sensitive"));
    }

    @Test
    void countsPaddingAndTruncation() {
        List<List<Integer>> sequences = List.of(List.of(2, 5, 7), List.of(3, 8, 1, 9, 4), List.of(6, 2));
        assertEquals(2 + 0 + 3, TokenizerExperiment.countPadding(sequences, 5));
        assertEquals(0, TokenizerExperiment.countTruncated(sequences, 5));
        assertEquals(1 + 3, TokenizerExperiment.countTruncated(sequences, 2));
    }

    @Test
    void otherExperimentsProduceTables() {
        assertTrue(experiment.minFrequencyExperiment(documents, new int[]{1, 2}).contains("minFrequency"));
        assertTrue(experiment.vocabularyLimitExperiment(documents, new int[]{3, 100}).contains("Max size"));
        assertTrue(experiment.paddingExperiment(documents, new int[]{5, 10}).contains("maxLength"));
    }
}
