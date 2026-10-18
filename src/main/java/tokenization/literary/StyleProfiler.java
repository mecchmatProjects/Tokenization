package tokenization.literary;

import tokenization.core.Document;
import tokenization.core.Token;
import tokenization.core.TokenType;
import tokenization.normalization.Lemmatizer;
import tokenization.normalization.Normalizer;
import tokenization.preprocessing.TextPreprocessor;
import tokenization.tokenizer.RuleBasedTokenizer;
import tokenization.tokenizer.Tokenizer;

import java.util.ArrayList;
import java.util.List;

/**
 * Будує стильовий профіль тексту (README_LITCRITIC §1):
 * попередня обробка → токенізація → лематизація → речення → аналізатори → профіль.
 */
public class StyleProfiler {

    private final TextPreprocessor preprocessor = new TextPreprocessor();
    private final Tokenizer tokenizer = new RuleBasedTokenizer();
    private final Normalizer normalizer = new Normalizer();
    private final Lemmatizer lemmatizer = new Lemmatizer();
    private final SentenceSplitter sentenceSplitter = new SentenceSplitter();
    private final List<TextAnalyzer> analyzers = new ArrayList<>();

    public StyleProfiler() {
        // різні лапки та тире зводимо до одного вигляду, щоб їх було простіше рахувати;
        // пунктуацію не видаляємо — вона важлива для аналізу стилю
        preprocessor.setNormalizeQuotes(true);
        preprocessor.setNormalizeDashes(true);

        analyzers.add(new LexicalAnalyzer());
        analyzers.add(new FrequencyAnalyzer());
        analyzers.add(new SentenceAnalyzer());
        analyzers.add(new PunctuationAnalyzer());
        analyzers.add(new RepetitionAnalyzer());
        analyzers.add(new NGramAnalyzer());
    }

    public AnalyzedText prepare(Document document) {
        String text = preprocessor.process(document.getContent());
        List<Token> tokens = normalizer.normalize(tokenizer.tokenize(text));
        for (Token token : tokens) {
            if (token.getType() == TokenType.WORD) {
                token.setLemma(lemmatizer.lemmatize(token.getNormalizedText()));
            }
        }
        List<Sentence> sentences = sentenceSplitter.split(tokens, text);
        return new AnalyzedText(document, text, tokens, sentences);
    }

    public StyleProfile buildProfile(AnalyzedText text) {
        Document document = text.getDocument();
        StyleProfile profile = new StyleProfile(document.getTitle(), document.getAuthor(), document.getYear());
        profile.characters = document.getContent().length();
        profile.sentenceCount = text.getSentences().size();

        for (TextAnalyzer analyzer : analyzers) {
            analyzer.analyze(text, profile);
        }
        return profile;
    }

    public StyleProfile analyze(Document document) {
        return buildProfile(prepare(document));
    }
}
