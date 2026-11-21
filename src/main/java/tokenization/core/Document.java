package tokenization.core;

/**
 * Текстовий документ, завантажений з файлу, разом з метаданими (README_LITCRITIC §3).
 */
public class Document {

    private final int id;
    private final String name;
    private final String content;

    private String title = "";
    private String author = "";
    private int year;
    private String language = "";

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

    /** Назва твору; якщо її не задано — назва файлу без розширення. */
    public String getTitle() {
        if (!title.isEmpty()) {
            return title;
        }
        return name.endsWith(".txt") ? name.substring(0, name.length() - 4) : name;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getAuthor() {
        return author;
    }

    public void setAuthor(String author) {
        this.author = author;
    }

    /** Рік написання; 0, якщо невідомий. */
    public int getYear() {
        return year;
    }

    public void setYear(int year) {
        this.year = year;
    }

    public String getLanguage() {
        return language;
    }

    public void setLanguage(String language) {
        this.language = language;
    }
}
