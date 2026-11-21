package tokenization.literary;

/**
 * Спільний інтерфейс аналізаторів тексту (README_LITCRITIC §24).
 * Кожен аналізатор обчислює свою групу характеристик і записує їх у стильовий профіль.
 */
public interface TextAnalyzer {

    void analyze(AnalyzedText text, StyleProfile profile);
}
