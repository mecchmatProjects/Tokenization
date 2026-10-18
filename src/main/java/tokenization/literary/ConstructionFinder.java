package tokenization.literary;

import java.util.ArrayList;
import java.util.List;

/**
 * Пошук характерних мовних конструкцій за шаблоном (README_LITCRITIC §17),
 * наприклад "якщо ... то ...", "не тільки ... а й ...".
 *
 * "..." означає будь-яку кількість слів. Частини шаблону мають іти в тому самому порядку
 * в межах одного речення. Регістр не враховується.
 */
public class ConstructionFinder {

    public List<Sentence> find(AnalyzedText text, String pattern) {
        List<List<String>> parts = parsePattern(pattern);
        List<Sentence> result = new ArrayList<>();
        if (parts.isEmpty()) {
            return result;
        }
        for (Sentence sentence : text.getSentences()) {
            if (matches(sentence.getWords(), parts)) {
                result.add(sentence);
            }
        }
        return result;
    }

    /** "не тільки ... а й ..." → [[не, тільки], [а, й]] */
    static List<List<String>> parsePattern(String pattern) {
        List<List<String>> parts = new ArrayList<>();
        for (String part : pattern.toLowerCase().split("\\.\\.\\.|…")) {
            List<String> words = new ArrayList<>();
            for (String word : part.trim().split("\\s+")) {
                if (!word.isEmpty()) {
                    words.add(word);
                }
            }
            if (!words.isEmpty()) {
                parts.add(words);
            }
        }
        return parts;
    }

    private boolean matches(List<String> words, List<List<String>> parts) {
        int position = 0;
        for (List<String> part : parts) {
            int found = indexOf(words, part, position);
            if (found < 0) {
                return false;
            }
            position = found + part.size();
        }
        return true;
    }

    /** Перша позиція (не раніше from), з якої в words іде послідовність part. */
    private int indexOf(List<String> words, List<String> part, int from) {
        for (int i = from; i + part.size() <= words.size(); i++) {
            boolean equal = true;
            for (int j = 0; j < part.size(); j++) {
                if (!words.get(i + j).equals(part.get(j))) {
                    equal = false;
                    break;
                }
            }
            if (equal) {
                return i;
            }
        }
        return -1;
    }
}
