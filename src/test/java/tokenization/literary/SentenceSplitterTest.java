package tokenization.literary;

import org.junit.jupiter.api.Test;
import tokenization.core.Token;
import tokenization.tokenizer.RuleBasedTokenizer;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class SentenceSplitterTest {

    private List<Sentence> split(String text) {
        List<Token> tokens = new RuleBasedTokenizer().tokenize(text);
        return new SentenceSplitter().split(tokens, text);
    }

    private List<String> texts(String text) {
        List<String> result = new ArrayList<>();
        for (Sentence sentence : split(text)) {
            result.add(sentence.getText());
        }
        return result;
    }

    @Test
    void splitsBySentenceEndPunctuation() {
        assertEquals(List.of("Це був день.", "Чому?!", "Не знаю..."),
                texts("Це був день. Чому?! Не знаю..."));
    }

    @Test
    void oneSentenceWithoutFinalPunctuation() {
        assertEquals(List.of("Одне речення без крапки"), texts("Одне речення без крапки"));
    }

    @Test
    void abbreviationsDoNotEndSentence() {
        assertEquals(1, split("Ми купили хліб, молоко і т.д. на ринку.").size());
    }

    @Test
    void closingQuoteBelongsToSentence() {
        assertEquals(List.of("Він сказав: «Привіт!»", "Вона мовчала."),
                texts("Він сказав: «Привіт!» Вона мовчала."));
    }

    @Test
    void paragraphBreakEndsSentence() {
        assertEquals(List.of("Розділ перший", "Був ранок."), texts("Розділ перший\n\nБув ранок."));
    }

    @Test
    void countsWordsAndPunctuation() {
        Sentence sentence = split("Привіт, світе!").get(0);
        assertEquals(4, sentence.getTokenCount());
        assertEquals(2, sentence.getWordCount());
        assertEquals(2, sentence.getPunctuationCount());
    }

    @Test
    void setsSentenceNumbersInTokens() {
        List<Sentence> sentences = split("Раз. Два.");
        assertEquals(1, sentences.get(0).getTokens().get(0).getSentenceNumber());
        assertEquals(2, sentences.get(1).getTokens().get(0).getSentenceNumber());
    }

    @Test
    void emptyTextHasNoSentences() {
        assertTrue(split("").isEmpty());
    }
}
