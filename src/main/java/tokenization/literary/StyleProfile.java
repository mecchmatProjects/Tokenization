package tokenization.literary;

import tokenization.stats.Frequencies;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

/**
 * Стильовий профіль тексту — набір кількісних характеристик (README_LITCRITIC §18).
 * Поля заповнюються аналізаторами з цього пакета.
 */
public class StyleProfile {

    final String title;
    final String author;
    final int year;

    // загальна статистика
    int characters;
    int sentenceCount;

    // лексичний аналіз (§7-8)
    int wordCount;
    int uniqueWordCount;
    int uniqueLemmaCount;
    double lexicalDiversity;
    double averageWordLength;
    double averageSyllables;

    // частотний аналіз (§9)
    final Map<String, Integer> wordFrequencies = new HashMap<>();
    final Map<String, Integer> lemmaFrequencies = new HashMap<>();
    final Map<String, Integer> contentWordFrequencies = new HashMap<>();

    // довжини речень у словах (§11-12)
    double averageSentenceLength;
    double medianSentenceLength;
    int minSentenceLength;
    int maxSentenceLength;
    double sentenceLengthStdDev;
    // довжина речення → кількість таких речень, відсортовано за довжиною
    final Map<Integer, Integer> sentenceLengthDistribution = new TreeMap<>();

    // пунктуація (§13)
    final Map<String, Integer> punctuationFrequencies = new HashMap<>();
    double commasPerSentence;
    double punctuationPerSentence;

    // повторюваність слів (§14)
    int windowSize;
    int repeatedInWindow;
    int adjacentSentenceRepeats;
    int sentenceStartRepeats;
    double repetitionIndex;
    final Map<String, Integer> windowRepeatFrequencies = new HashMap<>();
    final Map<String, Integer> sentenceStartFrequencies = new HashMap<>();

    // повторюваність фраз (§15)
    final Map<String, Integer> bigramFrequencies = new HashMap<>();
    final Map<String, Integer> trigramFrequencies = new HashMap<>();
    int repeatedBigrams;
    int repeatedTrigrams;
    double phraseRepetition;

    public StyleProfile(String title, String author, int year) {
        this.title = title;
        this.author = author;
        this.year = year;
    }

    public String getTitle() {
        return title;
    }

    public String getAuthor() {
        return author;
    }

    public int getYear() {
        return year;
    }

    public int getCharacters() {
        return characters;
    }

    public int getSentenceCount() {
        return sentenceCount;
    }

    public int getWordCount() {
        return wordCount;
    }

    public int getUniqueWordCount() {
        return uniqueWordCount;
    }

    public int getUniqueLemmaCount() {
        return uniqueLemmaCount;
    }

    /** TTR = V / N. */
    public double getLexicalDiversity() {
        return lexicalDiversity;
    }

    public double getAverageWordLength() {
        return averageWordLength;
    }

    /** Середня кількість складів у слові (лише для слів кирилицею). */
    public double getAverageSyllables() {
        return averageSyllables;
    }

    public Map<String, Integer> getWordFrequencies() {
        return wordFrequencies;
    }

    public Map<String, Integer> getLemmaFrequencies() {
        return lemmaFrequencies;
    }

    public List<Map.Entry<String, Integer>> topWords(int n) {
        return Frequencies.top(wordFrequencies, n);
    }

    /** Найчастіші слова без stop words. */
    public List<Map.Entry<String, Integer>> topContentWords(int n) {
        return Frequencies.top(contentWordFrequencies, n);
    }

    public double getAverageSentenceLength() {
        return averageSentenceLength;
    }

    public double getMedianSentenceLength() {
        return medianSentenceLength;
    }

    public int getMinSentenceLength() {
        return minSentenceLength;
    }

    public int getMaxSentenceLength() {
        return maxSentenceLength;
    }

    public double getSentenceLengthStdDev() {
        return sentenceLengthStdDev;
    }

    public Map<Integer, Integer> getSentenceLengthDistribution() {
        return sentenceLengthDistribution;
    }

    public Map<String, Integer> getPunctuationFrequencies() {
        return punctuationFrequencies;
    }

    public int getPunctuationCount(String mark) {
        return punctuationFrequencies.getOrDefault(mark, 0);
    }

    public double getCommasPerSentence() {
        return commasPerSentence;
    }

    public double getPunctuationPerSentence() {
        return punctuationPerSentence;
    }

    public int getWindowSize() {
        return windowSize;
    }

    public int getRepeatedInWindow() {
        return repeatedInWindow;
    }

    public int getAdjacentSentenceRepeats() {
        return adjacentSentenceRepeats;
    }

    public int getSentenceStartRepeats() {
        return sentenceStartRepeats;
    }

    /** Частка змістових слів, які повторюють слово з вікна. */
    public double getRepetitionIndex() {
        return repetitionIndex;
    }

    public List<Map.Entry<String, Integer>> topWindowRepeats(int n) {
        return Frequencies.top(windowRepeatFrequencies, n);
    }

    public List<Map.Entry<String, Integer>> topSentenceStarts(int n) {
        return Frequencies.top(sentenceStartFrequencies, n);
    }

    public Map<String, Integer> getBigramFrequencies() {
        return bigramFrequencies;
    }

    public Map<String, Integer> getTrigramFrequencies() {
        return trigramFrequencies;
    }

    public List<Map.Entry<String, Integer>> topBigrams(int n) {
        return Frequencies.top(bigramFrequencies, n);
    }

    public List<Map.Entry<String, Integer>> topTrigrams(int n) {
        return Frequencies.top(trigramFrequencies, n);
    }

    public int getRepeatedBigrams() {
        return repeatedBigrams;
    }

    public int getRepeatedTrigrams() {
        return repeatedTrigrams;
    }

    /** Частка входжень біграм, які повторюються в тексті. */
    public double getPhraseRepetition() {
        return phraseRepetition;
    }
}
