#include <clocale>
#include <iostream>
#include <string>
#include <vector>

#include "lemmatizer.h"
#include "lexical_analyzer.h"
#include "preprocessor.h"
#include "report_generator.h"
#include "sentence_analyzer.h"
#include "style_profile.h"
#include "text_comparator.h"
#include "text_document.h"
#include "text_loader.h"
#include "tokenizer.h"

// Container for keeping document metadata together with its computed profile
struct AnalyzedDocument {
    std::string title;
    std::string author;
    StyleProfile profile;
};

// Run the full analysis pipeline on a single document and build its StyleProfile
StyleProfile analyzeDocument(
    const TextDocument& doc,
    const Lemmatizer& ukLemmatizer,
    const Lemmatizer& enLemmatizer
) {
    StyleProfile profile;

    // 1. Preprocessing and tokenization
    std::string cleanedText = Preprocessor::process(doc.getContent(), false, false);
    std::vector<Token> tokens = Tokenizer::tokenize(cleanedText);

    if (tokens.empty()) {
        return profile;
    }

    // 2. Lemmatization based on document language
    std::string lang = Preprocessor::convertToLowerCase(doc.getLanguage());
    if (lang == "en" || lang == "english") {
        enLemmatizer.lemmatizeTokens(tokens);
    } else {
        ukLemmatizer.lemmatizeTokens(tokens);
    }

    // 3. Lexical analysis
    LexicalAnalyzer lexicalAnalyzer;
    int totalWords = static_cast<int>(lexicalAnalyzer.countTotalWords(tokens));
    int uniqueWords = static_cast<int>(lexicalAnalyzer.countUniqueWords(tokens));
    double ttr = lexicalAnalyzer.calculateTTR(tokens);

    double totalWordChars = 0.0;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::WORD) {
            totalWordChars += static_cast<double>(token.getText().length());
        }
    }
    double avgWordLen = (totalWords > 0) ? (totalWordChars / totalWords) : 0.0;

    profile.setLexicalStats(totalWords, uniqueWords, uniqueWords, ttr, avgWordLen);

    // 4. Sentence and punctuation analysis
    SentenceAnalyzer sentenceAnalyzer;
    double avgSentLen = sentenceAnalyzer.calculateAverageSentenceLength(tokens);
    int maxSentLen = static_cast<int>(sentenceAnalyzer.findMaxSentenceLength(tokens));

    profile.setSentenceStats(avgSentLen, 0.0);
    profile.setMaxSentenceLength(maxSentLen);

    auto punctFreq = sentenceAnalyzer.analyzePunctuationFrequency(tokens);
    for (const auto& entry : punctFreq) {
        profile.addPunctuationStat(entry.first, static_cast<int>(entry.second));
    }

    return profile;
}

// Find document index by matching substrings in title and author
int findDocumentIndex(
    const std::vector<AnalyzedDocument>& docs,
    const std::string& titlePart,
    const std::string& authorPart
) {
    for (size_t i = 0; i < docs.size(); ++i) {
        bool titleMatch = docs[i].title.find(titlePart) != std::string::npos;
        bool authorMatch = docs[i].author.find(authorPart) != std::string::npos;
        if (titleMatch && authorMatch) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Compare two analyzed documents and print the formatted comparison report
void runComparison(
    const std::vector<AnalyzedDocument>& docs,
    int firstIdx,
    int secondIdx,
    const std::string& experimentTitle
) {
    if (firstIdx < 0 || secondIdx < 0) {
        std::cout << "Skipping comparison (" << experimentTitle
                  << "): one of the documents was not found.\n\n";
        return;
    }

    const AnalyzedDocument& first = docs[firstIdx];
    const AnalyzedDocument& second = docs[secondIdx];

    std::cout << ">>> " << experimentTitle << "\n";
    std::cout << "    [1] " << first.title << " — " << first.author << "\n";
    std::cout << "    [2] " << second.title << " — " << second.author << "\n";

    litcritic::ComparisonWeights weights;
    auto comparison = litcritic::TextComparator::compare(first.profile, second.profile, weights);

    // Use short ASCII labels for table columns to avoid UTF-8 truncation in setw/substr
    litcritic::ReportGenerator::printComparisonReport(
        first.profile,
        second.profile,
        comparison,
        "Text [1]",
        "Text [2]",
        std::cout
    );
}

int main() {
    // Set global locale to Ukrainian UTF-8 for proper character handling
    std::setlocale(LC_ALL, "uk_UA.UTF-8");

    std::cout << "Starting LitCritic Literary Style Analyzer...\n\n";

    // Load lemmatization dictionaries
    Lemmatizer ukLemmatizer;
    if (!ukLemmatizer.loadDictionary("datasets/dictionaries/uk_dict.txt")) {
        std::cout << "Warning: Could not load UK dictionary!\n";
    }

    Lemmatizer enLemmatizer;
    if (!enLemmatizer.loadDictionary("datasets/dictionaries/en_dict.txt")) {
        std::cout << "Warning: Could not load EN dictionary!\n";
    }

    // Load all documents from the catalog
    const std::string catalogPath = "datasets/catalog.csv";
    std::vector<TextDocument> documents = TextLoader::loadFromCatalog(catalogPath);

    std::cout << "Loaded " << documents.size() << " documents from catalog.\n\n";

    std::vector<AnalyzedDocument> analyzedDocs;

    // Analyze each document and print its formatted report with title and author
    for (const auto& doc : documents) {
        StyleProfile profile = analyzeDocument(doc, ukLemmatizer, enLemmatizer);
        std::string fullLabel = doc.getTitle() + " (" + doc.getAuthor() + ")";

        litcritic::ReportGenerator::printProfileReport(profile, fullLabel, std::cout);

        if (profile.getTotalWords() > 0) {
            analyzedDocs.push_back({doc.getTitle(), doc.getAuthor(), profile});
        }
    }

    // --- Experiment 1: Different authors, same title ("Місто") ---
    int pidmohylnyIdx = findDocumentIndex(analyzedDocs, "Місто", "Підмогильний");
    int semenkoIdx = findDocumentIndex(analyzedDocs, "Місто", "Семенко");
    runComparison(
        analyzedDocs,
        pidmohylnyIdx,
        semenkoIdx,
        "EXPERIMENT 1: Different Authors, Same Title"
    );

    // --- Experiment 2: Same author (Hryhorii Skovoroda) ---
    int skovorodaBeeIdx = findDocumentIndex(analyzedDocs, "Бджола", "Сковорода");
    int skovorodaGardenIdx = findDocumentIndex(analyzedDocs, "Сад", "Сковорода");
    runComparison(
        analyzedDocs,
        skovorodaBeeIdx,
        skovorodaGardenIdx,
        "EXPERIMENT 2: Same Author (Skovoroda)"
    );

    // --- Experiment 3: Same author (William Shakespeare) ---
    int hamletIdx = findDocumentIndex(analyzedDocs, "Hamlet", "Shakespeare");
    int sonnetsIdx = findDocumentIndex(analyzedDocs, "Sonnetes", "Shakespeare");
    runComparison(
        analyzedDocs,
        hamletIdx,
        sonnetsIdx,
        "EXPERIMENT 3: Same Author (Shakespeare)"
    );

    // --- Experiment 4: Different eras (Baroque vs Futurism) ---
    runComparison(
        analyzedDocs,
        skovorodaGardenIdx,
        semenkoIdx,
        "EXPERIMENT 4: Different Eras (Skovoroda vs Semenko)"
    );

    // --- Experiment 5: Different eras & genres (1641 Philosophy vs 2017 AI Science) ---
    int descartesIdx = findDocumentIndex(analyzedDocs, "онтологічний", "Декарт");
    int tegmarkIdx = findDocumentIndex(analyzedDocs, "Життя 3.0", "Тегмарк");
    runComparison(
        analyzedDocs,
        descartesIdx,
        tegmarkIdx,
        "EXPERIMENT 5: Different Eras & Genres (Descartes 1641 vs Tegmark 2017)"
    );
}