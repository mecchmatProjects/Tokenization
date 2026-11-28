package tokenization.literary;

import java.util.List;
import java.util.Map;

/**
 * Прості текстові діаграми для консолі та звітів (README_LITCRITIC §12, §21).
 */
public class TextChart {

    private static final char BAR = '█';

    private TextChart() {
    }

    /**
     * Горизонтальна стовпчикова діаграма:
     * <pre>
     * день       ████████████ 12
     * ніч        ██████ 6
     * </pre>
     */
    public static String barChart(List<Map.Entry<String, Integer>> entries, int maxWidth) {
        int max = 0;
        for (Map.Entry<String, Integer> entry : entries) {
            max = Math.max(max, entry.getValue());
        }
        StringBuilder sb = new StringBuilder();
        for (Map.Entry<String, Integer> entry : entries) {
            sb.append(String.format("  %-18s %s %d%n", entry.getKey(), bar(entry.getValue(), max, maxWidth),
                    entry.getValue()));
        }
        return sb.toString();
    }

    /**
     * Гістограма довжин речень: довжини групуються в інтервали по bucketSize слів.
     * <pre>
     *   1-5     ██████████ 10
     *   6-10    ████ 4
     * </pre>
     */
    public static String histogram(Map<Integer, Integer> distribution, int bucketSize, int maxWidth) {
        if (distribution.isEmpty()) {
            return "  (no data)\n";
        }
        int maxLength = 0;
        for (int length : distribution.keySet()) {
            maxLength = Math.max(maxLength, length);
        }
        int bucketCount = (maxLength - 1) / bucketSize + 1;
        int[] buckets = new int[bucketCount];
        for (Map.Entry<Integer, Integer> entry : distribution.entrySet()) {
            buckets[(entry.getKey() - 1) / bucketSize] += entry.getValue();
        }

        int max = 0;
        for (int count : buckets) {
            max = Math.max(max, count);
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < bucketCount; i++) {
            String label = (i * bucketSize + 1) + "-" + ((i + 1) * bucketSize);
            sb.append(String.format("  %-8s %s %d%n", label, bar(buckets[i], max, maxWidth), buckets[i]));
        }
        return sb.toString();
    }

    private static String bar(int value, int max, int maxWidth) {
        int width = max > 0 ? (int) Math.round((double) value * maxWidth / max) : 0;
        if (value > 0 && width == 0) {
            width = 1;
        }
        return String.valueOf(BAR).repeat(width);
    }
}
