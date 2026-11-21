package tokenization.literary;

import tokenization.core.Token;
import tokenization.word.Hyphenator;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Лексичний аналіз (README_LITCRITIC §7-8): кількість слів, унікальних слів і лем,
 * лексичне різноманіття TTR = V / N, середня довжина слова та кількість складів.
 */
public class LexicalAnalyzer implements TextAnalyzer {

    @Override
    public void analyze(AnalyzedText text, StyleProfile profile) {
        List<Token> words = text.getWords();
        Set<String> uniqueWords = new HashSet<>();
        Set<String> uniqueLemmas = new HashSet<>();
        long letters = 0;
        int syllables = 0;
        int cyrillicWords = 0;

        for (Token word : words) {
            String value = word.getNormalizedText();
            uniqueWords.add(value);
            uniqueLemmas.add(word.getLemma());
            letters += countLetters(value);
            if (isCyrillic(value)) {
                syllables += Hyphenator.countSyllables(value);
                cyrillicWords++;
            }
        }

        int n = words.size();
        profile.wordCount = n;
        profile.uniqueWordCount = uniqueWords.size();
        profile.uniqueLemmaCount = uniqueLemmas.size();
        profile.lexicalDiversity = n > 0 ? (double) uniqueWords.size() / n : 0;
        profile.averageWordLength = n > 0 ? (double) letters / n : 0;
        profile.averageSyllables = cyrillicWords > 0 ? (double) syllables / cyrillicWords : 0;
    }

    // апостроф і дефіс не рахуються як літери
    private int countLetters(String word) {
        int count = 0;
        for (int i = 0; i < word.length(); i++) {
            if (Character.isLetter(word.charAt(i))) {
                count++;
            }
        }
        return count;
    }

    private boolean isCyrillic(String word) {
        for (int i = 0; i < word.length(); i++) {
            if (Character.UnicodeBlock.of(word.charAt(i)) == Character.UnicodeBlock.CYRILLIC) {
                return true;
            }
        }
        return false;
    }
}
