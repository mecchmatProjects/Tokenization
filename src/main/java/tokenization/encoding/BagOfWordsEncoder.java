package tokenization.encoding;

import tokenization.vocabulary.Vocabulary;

import java.util.List;

/**
 * Модель Bag-of-Words (README §23): вектор довжини словника,
 * у позиції i — скільки разів токен з ID i зустрічається в документі.
 * Токени, яких немає у словнику, рахуються в позиції <UNK>.
 */
public class BagOfWordsEncoder {

    private final Vocabulary vocabulary;

    public BagOfWordsEncoder(Vocabulary vocabulary) {
        this.vocabulary = vocabulary;
    }

    public int[] encode(List<String> tokens) {
        int[] vector = new int[vocabulary.size()];
        for (String token : tokens) {
            vector[vocabulary.getId(token)]++;
        }
        return vector;
    }
}
