package tokenization.literary;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import tokenization.core.Document;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class ReportAndComparisonTest {

    @TempDir
    Path tempDir;

    private final StyleProfiler profiler = new StyleProfiler();

    private StyleProfile profile(String title, String author, String text) {
        Document document = new Document(1, title + ".txt", text);
        document.setAuthor(author);
        return profiler.analyze(document);
    }

    private final StyleProfile shortStyle = profile("A", "Автор А", "Був ранок. Тихий ранок. Марко спав.");
    private final StyleProfile longStyle = profile("B", "Автор Б",
            "Дорога, що вела до міста, була довга, курна й гаряча, і кожен, хто йшов нею, втомлювався.");

    @Test
    void featureMapContainsAllGroups() {
        Map<String, Double> features = shortStyle.toFeatureMap();
        assertTrue(features.containsKey("TTR"));
        assertTrue(features.containsKey("Avg sentence length"));
        assertTrue(features.containsKey("Comma / sentence"));
        assertTrue(features.containsKey("Repetition index"));
        assertEquals(3.0, features.get("Sentences"));
    }

    @Test
    void comparisonShowsBothTextsAndDifference() {
        String table = new TextComparator().compare(shortStyle, longStyle);
        assertTrue(table.contains("A (Автор А)"));
        assertTrue(table.contains("B (Автор Б)"));
        assertTrue(table.contains("Avg sentence length"));
        assertTrue(table.contains("Top content words"));
    }

    @Test
    void comparisonUsesOnlySelectedGroups() {
        TextComparator comparator = new TextComparator();
        comparator.setLexical(false);
        comparator.setPunctuation(false);
        comparator.setRepetition(false);

        Map<String, Double> features = comparator.selectedFeatures(shortStyle);
        assertTrue(features.containsKey("Avg sentence length"));
        assertFalse(features.containsKey("TTR"));
        assertFalse(comparator.compare(shortStyle, longStyle).contains("Top content words"));
    }

    @Test
    void reportContainsAllSections() throws IOException {
        ReportGenerator generator = new ReportGenerator();
        String report = generator.generate(shortStyle);
        for (String section : List.of("GENERAL STATISTICS", "LEXICAL STATISTICS", "SENTENCE STATISTICS",
                "PUNCTUATION", "REPETITION")) {
            assertTrue(report.contains(section), section);
        }
        assertTrue(report.contains("Author: Автор А"));

        Path file = tempDir.resolve("reports/report.txt");
        generator.save(report, file);
        assertEquals(report, Files.readString(file));
    }

    @Test
    void barChartScalesToMaximum() {
        String chart = TextChart.barChart(List.of(Map.entry("a", 4), Map.entry("b", 2)), 8);
        String[] lines = chart.split("\n");
        assertTrue(lines[0].contains("████████ 4"));
        assertTrue(lines[1].contains("████ 2"));
    }

    @Test
    void histogramGroupsLengthsIntoBuckets() {
        String chart = TextChart.histogram(Map.of(1, 2, 4, 1, 7, 3), 5, 10);
        String[] lines = chart.split("\n");
        assertEquals(2, lines.length);
        assertTrue(lines[0].contains("1-5") && lines[0].endsWith(" 3"));
        assertTrue(lines[1].contains("6-10") && lines[1].endsWith(" 3"));
    }
}
