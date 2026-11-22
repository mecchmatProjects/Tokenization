#include "report_generator.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace litcritic {

    void ReportGenerator::printDivider(std::ostream& out, char symbol, std::size_t width) {
        out << std::string(width, symbol) << '\n';
    }

    void ReportGenerator::printHeader(std::ostream& out, const std::string& title, std::size_t width) {
        printDivider(out, '=', width);
        if (title.size() + 4 < width) {
            const std::size_t padLeft = (width - title.size()) / 2;
            out << std::string(padLeft, ' ') << title << '\n';
        }
        else {
            out << "  " << title << '\n';
        }
        printDivider(out, '=', width);
    }

    void ReportGenerator::printMetricRow(
        std::ostream& out,
        const std::string& label,
        const std::string& value,
        std::size_t width
    ) {
        const std::size_t labelWidth = (width > 24) ? (width - 22) : 20;
        out << "  " << std::left << std::setw(static_cast<int>(labelWidth)) << label
            << std::right << std::setw(18) << value << '\n';
    }

    std::string ReportGenerator::formatDouble(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    std::string ReportGenerator::buildProgressBar(double ratio, std::size_t barWidth) {
        const double clamped = std::max(0.0, std::min(1.0, ratio));
        const auto filled = static_cast<std::size_t>(clamped * static_cast<double>(barWidth) + 0.5);
        const std::size_t empty = (barWidth >= filled) ? (barWidth - filled) : 0;

        return "[" + std::string(filled, '#') + std::string(empty, '-') + "]";
    }

    void ReportGenerator::printProfileReport(
        const StyleProfile& profile,
        const std::string& documentTitle,
        std::ostream& out
    ) {
        constexpr std::size_t kWidth = 62;

        printHeader(out, "LITCRITIC :: STYLE PROFILE REPORT", kWidth);
        printMetricRow(out, "Document:", documentTitle, kWidth);
        printDivider(out, '-', kWidth);

        out << "  LEXICAL & SYNTACTIC METRICS\n";
        printDivider(out, '-', kWidth);

        printMetricRow(out, "Total Words:", std::to_string(profile.getTotalWords()), kWidth);

        const double diversityPct = profile.getLexicalDiversity() * 100.0;
        printMetricRow(
            out,
            "Lexical Diversity (TTR):",
            formatDouble(profile.getLexicalDiversity(), 3) + " (" + formatDouble(diversityPct, 1) + "%)",
            kWidth
        );

        printMetricRow(
            out,
            "Avg. Sentence Length:",
            formatDouble(profile.getAverageSentenceLength(), 2) + " words",
            kWidth
        );

        printDivider(out, '-', kWidth);
        out << "  RICHNESS SCALE: "
            << buildProgressBar(profile.getLexicalDiversity(), 24)
            << " " << formatDouble(diversityPct, 1) << "%\n";
        printDivider(out, '=', kWidth);
        out << '\n';
    }

    void ReportGenerator::printComparisonReport(
        const StyleProfile& first,
        const StyleProfile& second,
        const StyleComparisonResult& comparison,
        const std::string& firstTitle,
        const std::string& secondTitle,
        std::ostream& out
    ) {
        constexpr std::size_t kWidth = 68;

        printHeader(out, "LITCRITIC :: COMPARATIVE STYLE ANALYSIS", kWidth);

        // Заголовок таблиці порівняння
        out << "  " << std::left << std::setw(24) << "Metric"
            << std::right << std::setw(13) << firstTitle.substr(0, 12)
            << std::right << std::setw(13) << secondTitle.substr(0, 12)
            << std::right << std::setw(14) << "Diff" << '\n';
        printDivider(out, '-', kWidth);

        // Рядок 1: Кількість слів
        out << "  " << std::left << std::setw(24) << "Total Words"
            << std::right << std::setw(13) << first.getTotalWords()
            << std::right << std::setw(13) << second.getTotalWords()
            << std::right << std::setw(14) << comparison.wordCountDiff << '\n';

        // Рядок 2: Лексична різноманітність
        out << "  " << std::left << std::setw(24) << "Lexical Diversity"
            << std::right << std::setw(13) << formatDouble(first.getLexicalDiversity(), 3)
            << std::right << std::setw(13) << formatDouble(second.getLexicalDiversity(), 3)
            << std::right << std::setw(14) << formatDouble(comparison.lexicalDiversityDiff, 3) << '\n';

        // Рядок 3: Середня довжина речення
        out << "  " << std::left << std::setw(24) << "Avg. Sentence Length"
            << std::right << std::setw(13) << formatDouble(first.getAverageSentenceLength(), 2)
            << std::right << std::setw(13) << formatDouble(second.getAverageSentenceLength(), 2)
            << std::right << std::setw(14) << formatDouble(comparison.averageSentenceLengthDiff, 2) << '\n';

        printDivider(out, '-', kWidth);

        const double similarityPct = comparison.similarityScore * 100.0;
        printMetricRow(
            out,
            "Normalized Style Distance:",
            formatDouble(comparison.normalizedDistance, 4),
            kWidth
        );
        printMetricRow(
            out,
            "Overall Style Similarity:",
            formatDouble(similarityPct, 1) + "%",
            kWidth
        );

        out << "  SIMILARITY: "
            << buildProgressBar(comparison.similarityScore, 30)
            << " " << formatDouble(similarityPct, 1) << "%\n";

        printDivider(out, '=', kWidth);
        out << '\n';
    }

} // namespace litcritic