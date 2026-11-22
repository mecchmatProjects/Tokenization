#include "lemmatizer.h"
#include "preprocessor.h"
#include <fstream>
#include <sstream>

Lemmatizer::Lemmatizer() {}

Lemmatizer::Lemmatizer(const std::string& dictionaryPath) {
    loadDictionary(dictionaryPath);
}



// Parses a line formated as "wordForm lemma" and ignores empty or comment lines
bool Lemmatizer::parseDictionaryLine(const std::string& line, std::string& wordForm, std::string& lemma) {
    std::istringstream iss(line);
    if (!(iss >> wordForm >> lemma)) {
        return false;
    }

    // Ignore comment lines starting with '#'
    if (!wordForm.empty() && wordForm[0] == '#') {
        return false;
    }
    return true;
}



// Reads a dictionary file line by line and populates the hash map
bool Lemmatizer::loadDictionary(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::string wordForm;
    std::string lemma;

    while (std::getline(file, line)) {
        if (parseDictionaryLine(line, wordForm, lemma)) {
            addRule(wordForm, lemma);
        }
    }
    return true;
}



// Normalizes both strings to lowercase and inserts the pair into the ditionary
void Lemmatizer::addRule(const std::string& wordForm, const std::string& lemma) {
    if (wordForm.empty() || lemma.empty()) {
        return;
    }

    std::string normWord = Preprocessor::convertToLowerCase(wordForm);
    std::string normLemma = Preprocessor::convertToLowerCase(lemma);
    dictionary[normWord] = normLemma;
}



// Looks up the normalized word in the dictionary; returns the original word if not found
std::string Lemmatizer::lemmatizeWord(const std::string& word) const {
    if (word.empty()) {
        return "";
    }

    std::string normWord = Preprocessor::convertToLowerCase(word);
    auto it = dictionary.find(normWord);
    if (it != dictionary.end()) {
        return it->second;
    }
    return normWord;
}



// Lemmatizes a single Token if its type is WORD
void Lemmatizer::lemmatizeToken(Token& token) const {
    if (token.getType() != TokenType::WORD) {
        return;
    }

    std::string lemma = lemmatizeWord(token.getNormalizedText());
    token.setLemma(lemma);
}



// Iterates through all tokens and lemmatizes each one
void Lemmatizer::lemmatizeTokens(std::vector<Token>& tokens) const {
    for (Token& token : tokens) {
        lemmatizeToken(token);
    }
}



// Returns the total count of rules stored in the dictionary
size_t Lemmatizer::getDictionarySize() const {
    return dictionary.size();
}