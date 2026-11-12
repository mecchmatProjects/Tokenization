package tokenization.experiment;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import tokenization.core.Document;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class PerformanceExperimentTest {

    @TempDir
    Path tempDir;

    private final List<Document> source = List.of(new Document(1, "a", "Machine learning is amazing! I have 2 projects."));

    @Test
    void buildsTextOfRequestedSize() {
        String text = PerformanceExperiment.buildText(source, 1000);
        assertTrue(text.length() >= 1000);
        assertTrue(text.startsWith("Machine learning"));
    }

    @Test
    void measuresAllSizesAndSavesCsv() throws IOException {
        PerformanceExperiment experiment = new PerformanceExperiment();
        List<PerformanceResult> results = experiment.run(source, new int[]{4, 16});

        assertEquals(2, results.size());
        assertTrue(results.get(1).getTokens() > results.get(0).getTokens());

        Path file = tempDir.resolve("perf.csv");
        experiment.exportCsv(results, file);
        assertEquals(3, Files.readAllLines(file).size());
    }
}
