package tokenization.preprocessing;

import java.text.Normalizer;

/**
 * Попередня обробка тексту перед токенізацією (README §4, README_LITCRITIC §4).
 *
 * Регістр тут не змінюється: це робить етап нормалізації токенів,
 * щоб правила токенізатора бачили оригінальний текст.
 */
public class TextPreprocessor {

    private boolean removeControlChars = true;
    private boolean normalizeUnicode = true;
    private boolean removeExtraSpaces = true;
    private boolean joinLines = false;
    private boolean normalizeQuotes = false;
    private boolean normalizeDashes = false;

    public String process(String text) {
        if (text == null || text.isEmpty()) {
            return "";
        }

        String result = normalizeLineBreaks(text);
        if (removeControlChars) {
            result = removeControlChars(result);
        }
        if (normalizeUnicode) {
            // NFC: літера + комбінований знак → одна літера (и + ̆ → й)
            result = Normalizer.normalize(result, Normalizer.Form.NFC);
        }
        if (removeExtraSpaces) {
            result = removeExtraSpaces(result);
        }
        if (normalizeQuotes) {
            result = normalizeQuotes(result);
        }
        if (normalizeDashes) {
            result = normalizeDashes(result);
        }
        if (joinLines) {
            result = joinLines(result);
        }
        return result;
    }

    /** \r\n та \r → \n, більше двох порожніх рядків поспіль → один порожній рядок. */
    private String normalizeLineBreaks(String text) {
        String result = text.replace("\r\n", "\n").replace('\r', '\n');
        return result.replaceAll("\n{3,}", "\n\n");
    }

    private String removeControlChars(String text) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c == '\n' || c == '\t' || !Character.isISOControl(c)) {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    /** Усі види лапок → ", усі види апострофів → '. */
    private String normalizeQuotes(String text) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if ("«»„“”″".indexOf(c) >= 0) {
                sb.append('"');
            } else if ("‘’ʼ`´".indexOf(c) >= 0) {
                sb.append('\'');
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    /** Різні тире → «—»; дефіс, оточений пробілами, теж вважається тире. */
    private String normalizeDashes(String text) {
        String result = text.replace('–', '—').replace('―', '—');
        return result.replace(" - ", " — ");
    }

    /** Кілька пробілів/табуляцій → один пробіл, без пробілів на краях рядків. */
    private String removeExtraSpaces(String text) {
        String result = text.replace(' ', ' ');
        result = result.replaceAll("[ \\t]+", " ");
        result = result.replaceAll(" *\n *", "\n");
        return result.trim();
    }

    /** Одиночні перенесення рядків → пробіл, порожні рядки між абзацами зберігаються. */
    private String joinLines(String text) {
        String[] paragraphs = text.split("\n\n");
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < paragraphs.length; i++) {
            if (i > 0) {
                sb.append("\n\n");
            }
            sb.append(paragraphs[i].replace('\n', ' '));
        }
        return sb.toString();
    }

    public void setRemoveControlChars(boolean removeControlChars) {
        this.removeControlChars = removeControlChars;
    }

    public void setNormalizeUnicode(boolean normalizeUnicode) {
        this.normalizeUnicode = normalizeUnicode;
    }

    public void setRemoveExtraSpaces(boolean removeExtraSpaces) {
        this.removeExtraSpaces = removeExtraSpaces;
    }

    public void setJoinLines(boolean joinLines) {
        this.joinLines = joinLines;
    }

    public void setNormalizeQuotes(boolean normalizeQuotes) {
        this.normalizeQuotes = normalizeQuotes;
    }

    public void setNormalizeDashes(boolean normalizeDashes) {
        this.normalizeDashes = normalizeDashes;
    }

    public boolean isRemoveExtraSpaces() {
        return removeExtraSpaces;
    }

    public boolean isJoinLines() {
        return joinLines;
    }

    public boolean isNormalizeQuotes() {
        return normalizeQuotes;
    }

    public boolean isNormalizeDashes() {
        return normalizeDashes;
    }
}
