#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream> // Needed to create temporary test files
#include <cstdio> // Needed for std::remove to delete files

#include "text_loader.h"
#include "text_document.h"



void test_single_document_success() {
    // Test loading a real file that we know exists
    std::string path = "datasets/authors/freud_sexuality.txt";
    TextDocument doc = TextLoader::loadDocument(path, "Три нариси з теорії сексуальності", "Зигмунд Фройд", 1905, "uk");

    assert(doc.getTitle() == "Три нариси з теорії сексуальності");
    assert(doc.getAuthor() == "Зигмунд Фройд");
    assert(doc.getYear() == 1905);
    assert(doc.getLanguage() == "uk");
    // We check that the content is not empty (greater than 0 characters)
    assert(doc.getContent().length() > 0);

    std::cout << "test_single_document_success passed!\n";
}


void test_single_document_not_found() {
    // Edge case: file does not exist
    TextDocument doc = TextLoader::loadDocument("datasets/face_book.txt", "No Book", "Nobody", 0, "en");

    assert(doc.getTitle() == "No Book");
    // The program should not crash^ and content must be exactly empty
    assert(doc.getContent() == "");

    std::cout << "test_single_document_not_found passed!\n";
}


void test_catalog_success() {
    // Test loading the entire catalog with existing CSV
    std::vector<TextDocument> docs = TextLoader::loadFromCatalog("datasets/catalog.csv");
    assert(docs.size() >= 2);

    bool foundFreudSexuality = false;
    bool foundSkovorodaBee = false;

    for (const auto& doc : docs) {
        if (doc.getAuthor() == "Зигмунд Фройд" && doc.getTitle() == "Три нариси з теорії сексуальності") {
            assert(doc.getYear() == 1905);
            assert(doc.getContent().length() > 0);
            foundFreudSexuality = true;
        }
        else if (doc.getAuthor() == "Григорій Сковорода" && doc.getTitle() == "Бджола та шершень") {
            assert(doc.getYear() == 1774);
            foundSkovorodaBee = true;
        }
    }

    // Assert that both documents were actually found anywhere in the catalog
    assert(foundFreudSexuality);
    assert(foundSkovorodaBee);

    std::cout << "test_catalog_success passed!\n";
}

void test_catalog_not_found() {
    // Edge case: catalog file does not exist
    std::vector<TextDocument> docs = TextLoader::loadFromCatalog("datasets/face_catalog.csv");

    // The vector must be completly empty
    assert(docs.size() == 0);

    std::cout << "test_catalog_not_found passed!\n";
}

void test_catalog_malformed_year() {
    // Edge case: CSV has letters instead of numbers for the year
    // We create a temporary CSV file just for this test
    std::string tempPath = "datasets/temp_bad_catalog.csv";
    std::ofstream tempCsv(tempPath);
    if (tempCsv.is_open()) {
    // Use an existing text file so the content length is > 0
        tempCsv << "authors/freud_sexuality.txt;Bad Year Title;Some Author;INVALID_YEAR;en\n";        tempCsv.close();
    }

    std::vector<TextDocument> docs = TextLoader::loadFromCatalog(tempPath);

    assert(docs.size() == 1);
    // The try-catch block should catch the error and default the year to 0
    assert(docs[0].getYear() == 0); 
    assert(docs[0].getTitle() == "Bad Year Title");

    // Clean up the temporary file so it doesn't pollute folder
    std::remove(tempPath.c_str());

    std::cout << "test_catalog_malformed_year passed!\n";
}

void test_catalog_missing_columns() {
    // Edge case% CSV row is missing columns (a.g. year and language are absent)
    std::string tempPath = "datasets_missing_cols.csv";
    std::ofstream tempCsv(tempPath);
    if (tempCsv.is_open()) {
        tempCsv << "authors/dummy.txt;Incomplete Title;No Year Or Lang\n";
        tempCsv.close();
    }

    std::vector<TextDocument> docs = TextLoader::loadFromCatalog(tempPath);

    // The parser should skip the malformed row, so the vector must remain empty
    assert(docs.size() == 0);

    std::remove(tempPath.c_str());

    std::cout << "test_catalog_missing_columns passed!\n";
}



int main() {
    std::cout << "Running TextLoader tests ...\n";

    test_single_document_success();
    test_single_document_not_found();
    test_catalog_success();
    test_catalog_not_found();
    test_catalog_malformed_year();
    test_catalog_missing_columns();

    std::cout << "All TextLoader tests passed successfully!\n";
    return 0;
}