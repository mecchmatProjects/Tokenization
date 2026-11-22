#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "report_generator.h"
#include "style_profile.h"
#include "text_comparator.h"

using namespace litcritic;

void testSingleProfileReportOutput() {
    StyleProfile profile;
    profile.setTotalWords(250);
    profile.setLexicalStats(250, 150, 130, 0.60, 4.8);
    profile.setSentenceStats(14.5, 3.1);

    std::ostringstream buffer;
    ReportGenerator::printProfileReport(profile, "Chapter_01.txt", buffer);

    const std::string output = buffer.str();
    assert(output.find("LITCRITIC :: STYLE PROFILE REPORT") != std::string::npos);
    assert(output.find("Chapter_01.txt") != std::string::npos);
    assert(output.find("250") != std::string::npos);
    assert(output.find("0.600 (60.0%)") != std::string::npos);
    assert(output.find("14.50 words") != std::string::npos);
}

void testComparisonReportOutput() {
    StyleProfile p1;
    p1.setTotalWords(100);
    p1.setLexicalStats(100, 75, 70, 0.75, 4.3);
    p1.setSentenceStats(10.0, 2.0);

    StyleProfile p2;
    p2.setTotalWords(120);
    p2.setLexicalStats(120, 60, 55, 0.50, 5.1);
    p2.setSentenceStats(20.0, 4.5);

    StyleComparisonResult diff = TextComparator::compare(p1, p2);

    std::ostringstream buffer;
    ReportGenerator::printComparisonReport(p1, p2, diff, "Author_A", "Author_B", buffer);

    const std::string output = buffer.str();
    assert(output.find("COMPARATIVE STYLE ANALYSIS") != std::string::npos);
    assert(output.find("Author_A") != std::string::npos);
    assert(output.find("Author_B") != std::string::npos);
    assert(output.find("Overall Style Similarity:") != std::string::npos);
}

int main() {
    testSingleProfileReportOutput();
    testComparisonReportOutput();
    std::cout << "All ReportGenerator tests passed successfully!\n";
    return 0;
}