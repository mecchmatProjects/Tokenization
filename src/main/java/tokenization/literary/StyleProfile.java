package tokenization.literary;

import tokenization.stats.Frequencies;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

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

    // TODO: статистика довжин речень (§11-12)
    // TODO: статистика пунктуації (§13)
    // TODO: повторюваність слів і фраз (§14-16)

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
}
