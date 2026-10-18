package tokenization.preprocessing;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

class TextPreprocessorTest {

    @Test
    void removesExtraSpaces() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        assertEquals("Hello world", preprocessor.process("  Hello     world \t "));
    }

    @Test
    void keepsLineBreaksByDefault() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        assertEquals("Hello\nworld", preprocessor.process("Hello  \r\n  world"));
    }

    @Test
    void joinsLinesButKeepsParagraphs() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        preprocessor.setJoinLines(true);
        assertEquals("Hello world\n\nNew paragraph",
                preprocessor.process("Hello\nworld\n\n\n\nNew paragraph"));
    }

    @Test
    void joinsLinesFromLiteraryExample() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        preprocessor.setJoinLines(true);
        assertEquals("Це був дуже дивний день...",
                preprocessor.process("  Це   був\nдуже  дивний   день..."));
    }

    @Test
    void normalizesUnicodeToComposedForm() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        // "и" + комбінована бреве (U+0306) має стати однією літерою "й"
        String decomposed = "мой";
        assertEquals("мой", preprocessor.process(decomposed));
    }

    @Test
    void normalizesQuotesAndApostrophes() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        preprocessor.setNormalizeQuotes(true);
        assertEquals("\"Привіт\" - м'ята", preprocessor.process("«Привіт» - м’ята"));
    }

    @Test
    void normalizesDashes() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        preprocessor.setNormalizeDashes(true);
        assertEquals("Київ — столиця, 1990—2000, будь-який",
                preprocessor.process("Київ   -  столиця, 1990–2000, будь-який"));
    }

    @Test
    void removesControlCharacters() {
        TextPreprocessor preprocessor = new TextPreprocessor();
        assertEquals("ab", preprocessor.process("a\u0000b\u0007"));
    }

    @Test
    void handlesEmptyText() {
        assertEquals("", new TextPreprocessor().process(""));
        assertEquals("", new TextPreprocessor().process(null));
    }
}
