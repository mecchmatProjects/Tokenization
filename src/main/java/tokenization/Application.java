package tokenization;

import tokenization.core.Document;
import tokenization.pipeline.Dataset;
import tokenization.pipeline.ProcessedDocument;
import tokenization.pipeline.TextProcessor;
import tokenization.vocabulary.Vocabulary;

import java.util.Arrays;
import java.util.List;

/**
 * Точка входу в програму.
 */
public class Application {

    public static void main(String[] args) {
        System.out.println("=== Text Tokenizer ===");

        // TODO: замінити демонстрацію консольним меню (README §27)
        Document document = new Document(1, "example", "Machine learning is amazing! I have 2 projects.");
        TextProcessor processor = new TextProcessor();
        processor.setMaxLength(12);
        Dataset dataset = processor.process(List.of(document));

        Vocabulary vocabulary = dataset.getVocabulary();
        System.out.println("Vocabulary:");
        for (int id = 0; id < vocabulary.size(); id++) {
            System.out.println("  " + id + " -> " + vocabulary.getToken(id));
        }

        ProcessedDocument processed = dataset.getDocuments().get(0);
        System.out.println("Tokens:  " + processed.getNormalizedTexts());
        System.out.println("Encoded: " + processed.getEncoded());
        System.out.println("Padded:  " + Arrays.toString(processed.getPadded()));

        // TODO: статистика та експорт (README §25, §28)
    }
}
