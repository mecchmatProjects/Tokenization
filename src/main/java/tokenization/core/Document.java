package tokenization.core;

/**
 * Текстовий документ, завантажений з файлу.
 */
public class Document {

    private final int id;
    private final String name;
    private final String content;

    public Document(int id, String name, String content) {
        this.id = id;
        this.name = name;
        this.content = content;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public String getContent() {
        return content;
    }
}
