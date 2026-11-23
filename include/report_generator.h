#ifndef LITCRITIC_REPORT_GENERATOR_H
#define LITCRITIC_REPORT_GENERATOR_H

#include <iostream>
#include <ostream>
#include <string>

#include "style_profile.h"
#include "text_comparator.h"

namespace litcritic {

    class ReportGenerator {
    public:
        // Виводить звіт по одному стилістичному профілю
        static void printProfileReport(
            const StyleProfile& profile,
            const std::string& documentTitle = "Document Analysis",
            std::ostream& out = std::cout
        );

        // Виводить порівняльний звіт двох текстів
        static void printComparisonReport(
            const StyleProfile& first,
            const StyleProfile& second,
            const StyleComparisonResult& comparison,
            const std::string& firstTitle = "Text A",
            const std::string& secondTitle = "Text B",
            std::ostream& out = std::cout
        );

    private:
        static void printDivider(std::ostream& out, char symbol = '=', std::size_t width = 62);
        static void printHeader(std::ostream& out, const std::string& title, std::size_t width = 62);
        static void printMetricRow(
            std::ostream& out,
            const std::string& label,
            const std::string& value,
            std::size_t width = 62
        );
        static std::string formatDouble(double value, int precision = 2);
        static std::string buildProgressBar(double ratio, std::size_t barWidth = 20);
    };

} // namespace litcritic

#endif // LITCRITIC_REPORT_GENERATOR_H