package tokenization.tokenizer;

import tokenization.core.Token;
import tokenization.core.TokenType;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * Стратегія 3: токенізація на основі набору правил (README §11).
 *
 * Текст проходиться один раз зліва направо. У кожній позиції правила перевіряються
 * в такому порядку, і спрацьовує перше, що підійшло:
 *
 * 1. URL      — починається з http://, https:// або www. і триває до пробілу.
 * 2. EMAIL    — local@domain.zone, напр. test@example.com.
 * 3. DATE     — 12.05.2024, 1.5.2024, 12/05/2024, 2024-05-12.
 * 4. NUMBER   — [знак] цифри ([.,] цифри)*: 10, -15, 3.14, 1,000, 15,5.
 *               Знак "-"/"+" належить числу, лише якщо перед ним немає слова/числа (5-3 → 5 - 3).
 * 5. Скорочення — слова з відомого списку (Dr., etc., т.д.) та однолітерні частини з крапками (U.S.A., e.g.).
 * 6. WORD     — літери та цифри; апостроф і дефіс усередині слова не розривають його
 *               (don't, м'ята, mother-in-law, COVID-19); "++" після слова (C++).
 * 7. PUNCTUATION — "...", "?!", "--" як один токен (якщо groupPunctuation), інакше по одному символу.
 * 8. SPECIAL  — @ # $ % & * + = / \ _ | < > та інші символи.
 * 9. UNKNOWN  — усе інше.
 */
public class RuleBasedTokenizer implements Tokenizer {

    private static final Pattern EMAIL = Pattern.compile("[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)+");

    private static final Set<String> ABBREVIATIONS = Set.of(
            "mr.", "mrs.", "ms.", "dr.", "prof.", "etc.", "vs.", "inc.", "jr.", "sr.", "st.",
            "р.", "рр.", "ст.", "ім.", "проф.", "див.", "напр.", "тис.", "млн.", "грн.", "вул.", "обл."
    );

    // символи, які не можуть стояти в кінці URL чи email (кінець речення, дужки, лапки)
    private static final String TRAILING_CHARS = ".,!?;:)]}\"'»";

    private boolean groupPunctuation = true;

    @Override
    public List<Token> tokenize(String text) {
        List<Token> tokens = new ArrayList<>();
        int i = 0;

        while (i < text.length()) {
            char c = text.charAt(i);
            if (Character.isWhitespace(c)) {
                i++;
                continue;
            }

            int end = matchUrl(text, i);
            if (end > i) {
                i = addToken(tokens, text, i, end, TokenType.URL);
                continue;
            }
            end = matchEmail(text, i);
            if (end > i) {
                i = addToken(tokens, text, i, end, TokenType.EMAIL);
                continue;
            }
            end = matchDate(text, i);
            if (end > i) {
                i = addToken(tokens, text, i, end, TokenType.DATE);
                continue;
            }
            end = matchNumber(text, i);
            if (end > i) {
                i = addToken(tokens, text, i, end, TokenType.NUMBER);
                continue;
            }
            end = matchAbbreviation(text, i);
            if (end > i) {
                i = addToken(tokens, text, i, end, TokenType.WORD);
                continue;
            }
            if (CharUtils.isWordChar(c)) {
                i = addToken(tokens, text, i, matchWord(text, i), TokenType.WORD);
            } else if (CharUtils.isPunctuation(c)) {
                end = groupPunctuation ? CharUtils.punctuationGroupEnd(text, i) : i + 1;
                i = addToken(tokens, text, i, end, TokenType.PUNCTUATION);
            } else if (CharUtils.isSpecial(c)) {
                i = addToken(tokens, text, i, i + 1, TokenType.SPECIAL);
            } else {
                i = addToken(tokens, text, i, i + 1, TokenType.UNKNOWN);
            }
        }
        return tokens;
    }

    /** Додає токен і повертає позицію, з якої треба продовжити. */
    private int addToken(List<Token> tokens, String text, int start, int end, TokenType type) {
        tokens.add(new Token(text.substring(start, end), type, start));
        return end;
    }

    // ---------- Правила. Кожне повертає кінець знайденого токена або -1 ----------

    private int matchUrl(String text, int start) {
        if (!text.startsWith("http://", start) && !text.startsWith("https://", start)
                && !text.startsWith("www.", start)) {
            return -1;
        }
        int end = start;
        while (end < text.length() && !Character.isWhitespace(text.charAt(end))) {
            end++;
        }
        return stripTrailing(text, start, end);
    }

    private int matchEmail(String text, int start) {
        if (!Character.isLetterOrDigit(text.charAt(start))) {
            return -1;
        }
        int end = start;
        while (end < text.length() && isEmailChar(text.charAt(end))) {
            end++;
        }
        end = stripTrailing(text, start, end);
        String candidate = text.substring(start, end);
        if (candidate.indexOf('@') < 0 || !EMAIL.matcher(candidate).matches()) {
            return -1;
        }
        return end;
    }

    private boolean isEmailChar(char c) {
        return Character.isLetterOrDigit(c) || "._%+-@".indexOf(c) >= 0;
    }

    private int stripTrailing(String text, int start, int end) {
        while (end > start && TRAILING_CHARS.indexOf(text.charAt(end - 1)) >= 0) {
            end--;
        }
        return end;
    }

    /** Дата з трьох груп цифр з однаковим роздільником: dd.mm.yyyy або yyyy-mm-dd. */
    private int matchDate(String text, int start) {
        int first = countDigits(text, start);
        if (first == 0 || first == 3 || first > 4) {
            return -1;
        }
        int pos = start + first;
        if (pos >= text.length() || ".-/".indexOf(text.charAt(pos)) < 0) {
            return -1;
        }
        char separator = text.charAt(pos);
        pos++;

        int second = countDigits(text, pos);
        if (second < 1 || second > 2) {
            return -1;
        }
        pos += second;
        if (pos >= text.length() || text.charAt(pos) != separator) {
            return -1;
        }
        pos++;

        int third = countDigits(text, pos);
        boolean yearFirst = first == 4 && (third == 1 || third == 2);
        boolean yearLast = first <= 2 && (third == 2 || third == 4);
        if (!yearFirst && !yearLast) {
            return -1;
        }
        pos += third;
        if (pos < text.length() && CharUtils.isWordChar(text.charAt(pos))) {
            return -1;
        }
        return pos;
    }

    private int matchNumber(String text, int start) {
        int pos = start;
        char c = text.charAt(pos);
        if ((c == '-' || c == '+') && canStartSignedNumber(text, start)) {
            pos++;
        }
        int digits = countDigits(text, pos);
        if (digits == 0) {
            return -1;
        }
        pos += digits;

        // "2nd", "3D", "19-річний" — це слова, а не числа
        if (pos < text.length() && Character.isLetter(text.charAt(pos))) {
            return -1;
        }
        if (pos + 1 < text.length() && text.charAt(pos) == '-' && Character.isLetter(text.charAt(pos + 1))) {
            return -1;
        }

        // дробова частина або роздільник тисяч: 3.14, 15,5, 1,000,000
        while (pos + 1 < text.length() && (text.charAt(pos) == '.' || text.charAt(pos) == ',')
                && Character.isDigit(text.charAt(pos + 1))) {
            pos++;
            pos += countDigits(text, pos);
        }
        return pos;
    }

    /** Знак належить числу, якщо після нього цифра, а перед ним немає слова, числа чи дужки. */
    private boolean canStartSignedNumber(String text, int signPos) {
        if (signPos + 1 >= text.length() || !Character.isDigit(text.charAt(signPos + 1))) {
            return false;
        }
        if (signPos == 0) {
            return true;
        }
        char prev = text.charAt(signPos - 1);
        return !CharUtils.isWordChar(prev) && prev != ')' && prev != ']';
    }

    private int matchAbbreviation(String text, int start) {
        if (!Character.isLetter(text.charAt(start))) {
            return -1;
        }

        // однолітерні частини з крапками: U.S.A., e.g., т.д.
        int pos = start;
        int parts = 0;
        while (pos + 1 < text.length() && Character.isLetter(text.charAt(pos)) && text.charAt(pos + 1) == '.') {
            pos += 2;
            parts++;
        }
        if (parts >= 2) {
            return pos;
        }

        // слова зі списку відомих скорочень: Dr., etc., проф.
        int end = start;
        while (end < text.length() && Character.isLetter(text.charAt(end))) {
            end++;
        }
        if (end < text.length() && text.charAt(end) == '.') {
            String candidate = text.substring(start, end + 1).toLowerCase();
            if (ABBREVIATIONS.contains(candidate)) {
                return end + 1;
            }
        }
        return -1;
    }

    private int matchWord(String text, int start) {
        int end = start;
        while (end < text.length()) {
            char c = text.charAt(end);
            if (CharUtils.isWordChar(c)) {
                end++;
            } else if ((CharUtils.isApostrophe(c) || c == '-') && end > start
                    && end + 1 < text.length() && CharUtils.isWordChar(text.charAt(end + 1))) {
                // апостроф або дефіс між двома символами слова
                end++;
            } else {
                break;
            }
        }

        // C++, g++
        if (text.startsWith("++", end)) {
            int after = end + 2;
            if (after >= text.length() || !CharUtils.isWordChar(text.charAt(after))) {
                end = after;
            }
        }
        return end;
    }

    private int countDigits(String text, int start) {
        int count = 0;
        while (start + count < text.length() && Character.isDigit(text.charAt(start + count))) {
            count++;
        }
        return count;
    }

    public void setGroupPunctuation(boolean groupPunctuation) {
        this.groupPunctuation = groupPunctuation;
    }

    @Override
    public String getName() {
        return "Rule-based";
    }
}
