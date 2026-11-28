package tokenization.ui;

import tokenization.core.Document;
import tokenization.io.TextLoader;
import tokenization.literary.AnalyzedText;
import tokenization.literary.ConstructionFinder;
import tokenization.literary.ReportGenerator;
import tokenization.literary.Sentence;
import tokenization.literary.StyleProfile;
import tokenization.literary.StyleProfiler;
import tokenization.literary.TextChart;
import tokenization.literary.TextComparator;

import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Консольне меню літературного аналізу (README_LITCRITIC §25).
 */
public class LiteraryMenu {

    private static final int TOP_COUNT = 20;

    private final ConsoleInput input;
    private final StyleProfiler profiler = new StyleProfiler();

    private List<Document> documents = new ArrayList<>();
    private List<AnalyzedText> texts = new ArrayList<>();
    private List<StyleProfile> profiles = new ArrayList<>();

    public LiteraryMenu(ConsoleInput input) {
        this.input = input;
    }

    public void run() {
        while (true) {
            System.out.println();
            System.out.println("=================================");
            System.out.println(" Literary Text Analyzer");
            System.out.println("=================================");
            System.out.println(" 1. Load text");
            System.out.println(" 2. Analyze text");
            System.out.println(" 3. Show lexical statistics");
            System.out.println(" 4. Show sentence statistics");
            System.out.println(" 5. Show word frequencies");
            System.out.println(" 6. Show repeated phrases");
            System.out.println(" 7. Build style profile");
            System.out.println(" 8. Compare texts");
            System.out.println(" 9. Generate report");
            System.out.println("10. Search constructions");
            System.out.println(" 0. Back");

            int choice = input.readChoice();
            switch (choice) {
                case 1 -> load();
                case 2 -> analyze();
                case 3 -> showLexical();
                case 4 -> showSentences();
                case 5 -> showFrequencies();
                case 6 -> showRepetitions();
                case 7 -> showProfile();
                case 8 -> compare();
                case 9 -> report();
                case 10 -> searchConstructions();
                case 0 -> {
                    return;
                }
                default -> System.out.println("Unknown option.");
            }
        }
    }

    private void load() {
        String path = input.readLine("File or directory", "texts");
        try {
            documents = new TextLoader().load(Path.of(path));
            texts = new ArrayList<>();
            profiles = new ArrayList<>();
            System.out.println("Loaded texts: " + documents.size());
            printDocumentList();
        } catch (IOException e) {
            System.out.println("Cannot load: " + e.getMessage());
        }
    }

    private void printDocumentList() {
        for (int i = 0; i < documents.size(); i++) {
            Document document = documents.get(i);
            String author = document.getAuthor().isEmpty() ? "" : " — " + document.getAuthor();
            System.out.println("  " + (i + 1) + ". " + document.getTitle() + author);
        }
    }

    private void analyze() {
        if (documents.isEmpty()) {
            System.out.println("Load texts first.");
            return;
        }
        texts = new ArrayList<>();
        profiles = new ArrayList<>();
        for (Document document : documents) {
            AnalyzedText text = profiler.prepare(document);
            texts.add(text);
            profiles.add(profiler.buildProfile(text));
        }
        System.out.printf("%-4s %-24s %-14s %8s %10s %8s%n", "#", "Title", "Author", "Words", "Sentences", "TTR");
        for (int i = 0; i < profiles.size(); i++) {
            StyleProfile p = profiles.get(i);
            System.out.printf("%-4d %-24s %-14s %8d %10d %8.3f%n", i + 1, p.getTitle(), p.getAuthor(),
                    p.getWordCount(), p.getSentenceCount(), p.getLexicalDiversity());
        }
    }

    /** Номер тексту для перегляду; -1, якщо тексти ще не проаналізовано. */
    private int selectText(String prompt) {
        if (profiles.isEmpty()) {
            System.out.println("Analyze texts first.");
            return -1;
        }
        if (profiles.size() == 1) {
            return 0;
        }
        printDocumentList();
        int index = input.readInt(prompt, 1) - 1;
        if (index < 0 || index >= profiles.size()) {
            System.out.println("No such text.");
            return -1;
        }
        return index;
    }

    private void showLexical() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        StyleProfile p = profiles.get(index);
        System.out.printf("Total words:          %10d%n", p.getWordCount());
        System.out.printf("Unique words:         %10d%n", p.getUniqueWordCount());
        System.out.printf("Unique lemmas:        %10d%n", p.getUniqueLemmaCount());
        System.out.printf("TTR:                  %10.3f%n", p.getLexicalDiversity());
        System.out.printf("Average word length:  %10.2f%n", p.getAverageWordLength());
        System.out.printf("Average syllables:    %10.2f%n", p.getAverageSyllables());
    }

    private void showSentences() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        StyleProfile p = profiles.get(index);
        System.out.printf("Sentences:                %8d%n", p.getSentenceCount());
        System.out.printf("Average sentence length:  %8.1f words%n", p.getAverageSentenceLength());
        System.out.printf("Median:                   %8.1f%n", p.getMedianSentenceLength());
        System.out.printf("Minimum:                  %8d%n", p.getMinSentenceLength());
        System.out.printf("Maximum:                  %8d%n", p.getMaxSentenceLength());
        System.out.printf("Std deviation:            %8.1f%n", p.getSentenceLengthStdDev());
        System.out.println("\nWords per sentence:");
        System.out.print(TextChart.histogram(p.getSentenceLengthDistribution(), 5, 40));
    }

    private void showFrequencies() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        StyleProfile p = profiles.get(index);
        System.out.println("Top " + TOP_COUNT + " words:");
        System.out.print(TextChart.barChart(p.topWords(TOP_COUNT), 30));
        System.out.println("\nTop " + TOP_COUNT + " content words:");
        System.out.print(TextChart.barChart(p.topContentWords(TOP_COUNT), 30));
    }

    private void showRepetitions() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        StyleProfile p = profiles.get(index);
        System.out.println("Repeats within " + p.getWindowSize() + " words: " + p.getRepeatedInWindow());
        System.out.println("Repeats in adjacent sentences: " + p.getAdjacentSentenceRepeats());
        System.out.println("Repeated sentence starts:      " + p.getSentenceStartRepeats());
        printTop("Words repeated within the window (number of repeats)", p.topWindowRepeats(TOP_COUNT), 1);
        printTop("Bigrams (frequency >= 2)", p.topBigrams(TOP_COUNT), 2);
        printTop("Trigrams (frequency >= 2)", p.topTrigrams(TOP_COUNT), 2);
    }

    private void printTop(String title, List<Map.Entry<String, Integer>> entries, int minValue) {
        System.out.println("\n" + title + ":");
        boolean any = false;
        for (Map.Entry<String, Integer> entry : entries) {
            if (entry.getValue() >= minValue) {
                System.out.printf("  %-30s %5d%n", entry.getKey(), entry.getValue());
                any = true;
            }
        }
        if (!any) {
            System.out.println("  (none)");
        }
    }

    private void showProfile() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        StyleProfile p = profiles.get(index);
        System.out.println(p.getTitle() + (p.getAuthor().isEmpty() ? "" : " — " + p.getAuthor()));
        for (Map.Entry<String, Double> entry : p.toFeatureMap().entrySet()) {
            System.out.printf("  %-28s %12.3f%n", entry.getKey(), entry.getValue());
        }
    }

    private void compare() {
        if (profiles.size() < 2) {
            System.out.println("Analyze at least two texts first.");
            return;
        }
        int first = selectText("Select first text");
        int second = first < 0 ? -1 : selectText("Select second text");
        if (first < 0 || second < 0) {
            return;
        }

        TextComparator comparator = new TextComparator();
        System.out.println("Select characteristics:");
        comparator.setLexical(input.readYesNo("  Lexical", true));
        comparator.setSentence(input.readYesNo("  Sentence", true));
        comparator.setPunctuation(input.readYesNo("  Punctuation", true));
        comparator.setRepetition(input.readYesNo("  Repetition", true));
        System.out.println();
        System.out.println(comparator.compare(profiles.get(first), profiles.get(second)));
        System.out.println("The table shows measured differences only; it does not judge which style is better.");
    }

    private void report() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        StyleProfile p = profiles.get(index);
        ReportGenerator generator = new ReportGenerator();
        String report = generator.generate(p);
        System.out.println(report);

        String defaultName = "output/report_" + p.getTitle().replaceAll("[^\\p{L}\\p{N}]+", "_") + ".txt";
        String path = input.readLine("Save report to (empty = don't save)", defaultName);
        if (path == null || path.isEmpty()) {
            return;
        }
        try {
            generator.save(report, Path.of(path));
            System.out.println("Saved to " + path);
        } catch (IOException e) {
            System.out.println("Cannot save: " + e.getMessage());
        }
    }

    private void searchConstructions() {
        int index = selectText("Text");
        if (index < 0) {
            return;
        }
        String pattern = input.readLine("Pattern", "якщо ... то ...");
        List<Sentence> found = new ConstructionFinder().find(texts.get(index), pattern);
        System.out.println("Found: " + found.size());
        for (int i = 0; i < Math.min(10, found.size()); i++) {
            System.out.println("  " + found.get(i).getNumber() + ". " + found.get(i).getText().replace('\n', ' '));
        }
        if (found.size() > 10) {
            System.out.println("  ...");
        }
    }
}
