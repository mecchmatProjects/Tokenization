package tokenization.encoding;

import tokenization.vocabulary.Vocabulary;

import java.util.ArrayList;
import java.util.List;

/**
 * Перетворення токенів на послідовність ID (README §20) та вирівнювання довжини (README §21).
 */
public class IntegerEncoder {

    private final Vocabulary vocabulary;
    private boolean addStartEnd = false;

    public IntegerEncoder(Vocabulary vocabulary) {
        this.vocabulary = vocabulary;
    }

    public List<Integer> encode(List<String> tokens) {
        List<Integer> ids = new ArrayList<>();
        if (addStartEnd) {
            ids.add(Vocabulary.START_ID);
        }
        for (String token : tokens) {
            ids.add(vocabulary.getId(token));
        }
        if (addStartEnd) {
            ids.add(Vocabulary.END_ID);
        }
        return ids;
    }

    public List<String> decode(List<Integer> ids) {
        List<String> tokens = new ArrayList<>();
        for (int id : ids) {
            tokens.add(vocabulary.getToken(id));
        }
        return tokens;
    }

    /**
     * Повертає масив довжини maxLength: довша послідовність обрізається з кінця,
     * коротша доповнюється <PAD> (ID 0) справа.
     */
    public static int[] pad(List<Integer> ids, int maxLength) {
        int[] result = new int[maxLength];
        for (int i = 0; i < maxLength; i++) {
            result[i] = i < ids.size() ? ids.get(i) : Vocabulary.PAD_ID;
        }
        return result;
    }

    public void setAddStartEnd(boolean addStartEnd) {
        this.addStartEnd = addStartEnd;
    }
}
