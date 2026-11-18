#include <cmath>
#include <string>
#include <vector>
#include "TestFramework.h"
#include "../include/Token.h"
#include "../include/Document.h"
#include "../include/Dataset.h"
#include "../include/MockData.h"
#include "../include/Vocabulary.h"
#include "../include/IntegerEncoder.h"
#include "../include/OneHotEncoder.h"
#include "../include/BagOfWordsEncoder.h"
#include "../include/TfIdfEncoder.h"
#include "../include/StatisticsAnalyzer.h"
#include "../include/Application.h"

REGISTER_TEST(TextEdgeCases, EmptyTextAndWhitespaceOnly) {
    PipelineConfig cfg;

    Application app(cfg);
    PipelineResult emptyRes = app.runOnText("");
    ASSERT_TRUE(emptyRes.tokenFrequencies.empty());
    ASSERT_EQ(static_cast<size_t>(0), app.getDataset().getDocuments()[0].tokens.size());

    PipelineResult spacesRes = app.runOnText("   \t\n\r   ");
    ASSERT_TRUE(spacesRes.tokenFrequencies.empty());
    ASSERT_EQ(static_cast<size_t>(0), app.getDataset().getDocuments()[0].tokens.size());

    Dataset ds;
    Document emptyDoc;
    emptyDoc.id = 1;
    emptyDoc.rawText = "";
    ds.addDocument(emptyDoc);

    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);
    ASSERT_EQ(static_cast<size_t>(4), vocab.size());

    IntegerEncoder intEnc(false, 0);
    intEnc.encodeDataset(ds, vocab);
    ASSERT_TRUE(ds.getDocuments()[0].integerSequence.empty());
}

REGISTER_TEST(TextEdgeCases, PunctuationHandling) {
    PipelineConfig cfg;
    cfg.removePunctuation = false;

    Application appWithPunct(cfg);
    PipelineResult resWithPunct = appWithPunct.runOnText("Hello, world! Ready?");
    ASSERT_EQ(static_cast<size_t>(6), appWithPunct.getDataset().getDocuments()[0].tokens.size());

    cfg.removePunctuation = true;
    Application appNoPunct(cfg);
    PipelineResult resNoPunct = appNoPunct.runOnText("Hello, world! Ready?");
    ASSERT_EQ(static_cast<size_t>(3), appNoPunct.getDataset().getDocuments()[0].tokens.size());

    Token comma(",", TokenType::Punctuation, 1, 0);
    ASSERT_EQ(std::string("PUNCTUATION"), tokenTypeToString(comma.type));
}

REGISTER_TEST(TextEdgeCases, SpecialConstructsPreservation) {
    Dataset ds;
    Document doc;
    doc.id = 1;
    doc.tokens = {
        Token("email@example.com", TokenType::Special, 0, 0),
        Token("#machinelearning", TokenType::Special, 1, 0),
        Token("C++", TokenType::Special, 2, 0),
        Token("price=100$", TokenType::Special, 3, 0)
    };
    ds.addDocument(doc);

    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);
    ASSERT_EQ(static_cast<size_t>(8), vocab.size());
    ASSERT_TRUE(vocab.contains("email@example.com"));
    ASSERT_TRUE(vocab.contains("#machinelearning"));
    ASSERT_TRUE(vocab.contains("C++"));
    ASSERT_TRUE(vocab.contains("price=100$"));

    IntegerEncoder encoder(false, 0);
    encoder.encodeDataset(ds, vocab);
    auto decoded = encoder.decodeSequence(ds.getDocuments()[0].integerSequence, vocab);
    ASSERT_EQ(std::string("email@example.com"), decoded[0]);
    ASSERT_EQ(std::string("C++"), decoded[2]);

    PipelineConfig cfg;
    cfg.tokenizerType = TokenizerType::RuleBased;
    Application app(cfg);
    PipelineResult res = app.runOnText("Contact email@example.com #machinelearning C++ price=100$");
    ASSERT_FALSE(res.tokenFrequencies.empty());
}

REGISTER_TEST(TextEdgeCases, UnicodeAndCyrillicUtf8) {
    Dataset ds;
    Document doc;
    doc.id = 1;
    doc.rawText = u8"Привіт, світ! Токенізація UTF-8 працює.";
    doc.tokens = {
        Token(u8"Привіт", TokenType::Word, 0, 0),
        Token(u8"світ", TokenType::Word, 1, 0),
        Token(u8"Токенізація", TokenType::Word, 2, 1),
        Token("UTF-8", TokenType::Special, 3, 1),
        Token(u8"працює", TokenType::Word, 4, 1)
    };
    ds.addDocument(doc);

    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);
    ASSERT_EQ(static_cast<size_t>(9), vocab.size());
    ASSERT_TRUE(vocab.contains(u8"Токенізація"));

    IntegerEncoder encoder(false, 0);
    encoder.encodeDataset(ds, vocab);
    auto decoded = encoder.decodeSequence(ds.getDocuments()[0].integerSequence, vocab);
    ASSERT_EQ(std::string(u8"Привіт"), decoded[0]);
    ASSERT_EQ(std::string(u8"Токенізація"), decoded[2]);
    ASSERT_EQ(std::string(u8"працює"), decoded[4]);

    PipelineConfig cfg;
    Application app(cfg);
    PipelineResult res = app.runOnText(u8"Привіт, світ! Токенізація UTF-8 працює.");
    ASSERT_FALSE(res.tokenFrequencies.empty());
}

REGISTER_TEST(VocabularySuite, ReservedSpecialTokenIndices) {
    Vocabulary vocab;
    ASSERT_EQ(static_cast<size_t>(4), vocab.size());
    ASSERT_EQ(0, vocab.getTokenId("<PAD>"));
    ASSERT_EQ(1, vocab.getTokenId("<UNK>"));
    ASSERT_EQ(2, vocab.getTokenId("<START>"));
    ASSERT_EQ(3, vocab.getTokenId("<END>"));

    ASSERT_EQ(std::string("<PAD>"), vocab.getTokenText(0));
    ASSERT_EQ(std::string("<UNK>"), vocab.getTokenText(1));
    ASSERT_EQ(std::string("<START>"), vocab.getTokenText(2));
    ASSERT_EQ(std::string("<END>"), vocab.getTokenText(3));
    ASSERT_EQ(std::string(""), vocab.getTokenText(999));
    ASSERT_EQ(Vocabulary::UNK_ID, vocab.getTokenId("unseen_token"));
}

REGISTER_TEST(VocabularySuite, FrequencySortingAndFiltering) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);

    ASSERT_EQ(4, vocab.getTokenId("machine"));
    ASSERT_EQ(3, vocab.getTokenFrequency("machine"));
    ASSERT_EQ(static_cast<size_t>(22), vocab.size());

    Vocabulary filtered;
    filtered.buildFromDataset(ds, 2, 0);
    ASSERT_EQ(static_cast<size_t>(9), filtered.size());
    ASSERT_EQ(Vocabulary::UNK_ID, filtered.getTokenId("amazing"));
    ASSERT_EQ(13, filtered.getTokenFrequency("<UNK>"));
}

REGISTER_TEST(VocabularySuite, MaxVocabularySizeCapping) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary capped;
    capped.buildFromDataset(ds, 1, 7);

    ASSERT_EQ(static_cast<size_t>(7), capped.size());
    ASSERT_TRUE(capped.contains("machine"));
    ASSERT_TRUE(capped.contains("."));
    ASSERT_TRUE(capped.contains("data"));
    ASSERT_FALSE(capped.contains("learning"));
    ASSERT_EQ(17, capped.getTokenFrequency("<UNK>"));
}

REGISTER_TEST(IntegerEncoderSuite, BoundaryTokensAndDecoding) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);

    IntegerEncoder encoder(true, 0);
    encoder.encodeDataset(ds, vocab);

    const auto& seq = ds.getDocuments()[2].integerSequence;
    ASSERT_EQ(static_cast<size_t>(7), seq.size());
    ASSERT_EQ(Vocabulary::START_ID, seq.front());
    ASSERT_EQ(Vocabulary::END_ID, seq.back());

    auto decoded = encoder.decodeSequence(seq, vocab);
    ASSERT_EQ(std::string("<START>"), decoded.front());
    ASSERT_EQ(std::string("machine"), decoded[1]);
    ASSERT_EQ(std::string("<END>"), decoded.back());
}

REGISTER_TEST(IntegerEncoderSuite, PaddingAndTruncationAlignment) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);

    IntegerEncoder padEncoder(false, 12);
    padEncoder.encodeDataset(ds, vocab);

    ASSERT_EQ(static_cast<size_t>(12), ds.getDocuments()[0].integerSequence.size());
    ASSERT_EQ(Vocabulary::PAD_ID, ds.getDocuments()[0].integerSequence[10]);
    ASSERT_EQ(Vocabulary::PAD_ID, ds.getDocuments()[0].integerSequence[11]);
    ASSERT_EQ(static_cast<size_t>(12), IntegerEncoder::countDatasetPaddingTokens(ds));
    ASSERT_NEAR(12.0 / 36.0, IntegerEncoder::calculateDatasetPaddingRatio(ds), 1e-6);

    IntegerEncoder truncEncoder(false, 4);
    truncEncoder.encodeDataset(ds, vocab);
    ASSERT_EQ(static_cast<size_t>(4), ds.getDocuments()[0].integerSequence.size());
    ASSERT_EQ(static_cast<size_t>(0), IntegerEncoder::countPaddingTokens(ds.getDocuments()[0].integerSequence));
}

REGISTER_TEST(OneHotAndBoWSuite, BinaryPresenceAndSparsity) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);

    OneHotEncoder oh;
    oh.encodeDataset(ds, vocab);

    const auto& doc3 = ds.getDocuments()[2];
    ASSERT_EQ(vocab.size(), doc3.oneHotVector.size());

    int activeBits = 0;
    for (int bit : doc3.oneHotVector) {
        ASSERT_TRUE(bit == 0 || bit == 1);
        activeBits += bit;
    }
    ASSERT_EQ(5, activeBits);

    auto matrix = oh.encodeTokensMatrix(doc3.tokens, vocab);
    ASSERT_EQ(static_cast<size_t>(5), matrix.size());
    ASSERT_NEAR(21.0 / 22.0, OneHotEncoder::calculateMatrixSparsity(matrix), 1e-6);
}

REGISTER_TEST(OneHotAndBoWSuite, BagOfWordsFrequencyAccumulation) {
    Dataset ds;
    Document doc;
    doc.id = 1;
    doc.tokens = {
        Token("data", TokenType::Word, 0, 1),
        Token("science", TokenType::Word, 1, 1),
        Token("data", TokenType::Word, 2, 1)
    };
    ds.addDocument(doc);

    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);

    BagOfWordsEncoder bow;
    bow.encodeDataset(ds, vocab);

    const auto& vec = ds.getDocuments()[0].bowVector;
    int dataId = vocab.getTokenId("data");
    int scienceId = vocab.getTokenId("science");

    ASSERT_EQ(2, vec[dataId]);
    ASSERT_EQ(1, vec[scienceId]);
    ASSERT_EQ(3, BagOfWordsEncoder::getTotalTokenCount(vec));
    ASSERT_EQ(static_cast<size_t>(2), BagOfWordsEncoder::getNonZeroEntries(vec).size());
}

REGISTER_TEST(TfIdfSuite, MathematicalWeightsAndEmptyDocSafety) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary vocab;
    vocab.buildFromDataset(ds, 1, 0);

    TfIdfEncoder tfidf;
    tfidf.encodeDataset(ds, vocab);

    int machineId = vocab.getTokenId("machine");
    int fastId = vocab.getTokenId("fast");

    ASSERT_NEAR(0.0, ds.getDocuments()[0].tfidfVector[machineId], 1e-6);
    ASSERT_NEAR(0.0, ds.getDocuments()[2].tfidfVector[machineId], 1e-6);
    ASSERT_NEAR(0.2 * std::log(3.0), ds.getDocuments()[2].tfidfVector[fastId], 1e-5);

    Dataset emptyDocDs;
    Document emptyDoc;
    emptyDoc.id = 99;
    emptyDocDs.addDocument(emptyDoc);
    tfidf.encodeDataset(emptyDocDs, vocab);
    ASSERT_EQ(vocab.size(), emptyDocDs.getDocuments()[0].tfidfVector.size());
    for (double val : emptyDocDs.getDocuments()[0].tfidfVector) {
        ASSERT_FALSE(std::isnan(val));
        ASSERT_FALSE(std::isinf(val));
        ASSERT_NEAR(0.0, val, 1e-9);
    }
}

REGISTER_TEST(AnalyticsSuite, CorpusStatisticsAndUnknownRatio) {
    Dataset ds = MockData::createSampleDataset();
    Vocabulary filteredVocab;
    filteredVocab.buildFromDataset(ds, 2, 0);
    filteredVocab.assignIdsToDataset(ds);

    CorpusStatistics stats = StatisticsAnalyzer::analyze(ds, filteredVocab);
    ASSERT_EQ(static_cast<size_t>(3), stats.documentCount);
    ASSERT_EQ(static_cast<size_t>(24), stats.totalTokens);
    ASSERT_EQ(static_cast<size_t>(18), stats.uniqueTokens);
    ASSERT_EQ(static_cast<size_t>(5), stats.inVocabUniqueTokens);
    ASSERT_EQ(static_cast<size_t>(13), stats.unknownTokenOccurrences);
    ASSERT_NEAR(13.0 / 24.0, stats.unknownTokenRatio, 1e-6);
    ASSERT_NEAR(8.0, stats.avgDocumentLengthTokens, 1e-6);
}

REGISTER_TEST(AnalyticsSuite, DatasetMiniBatchSplitting) {
    Dataset ds = MockData::createSampleDataset();
    auto batches = ds.splitIntoBatches(2);
    ASSERT_EQ(static_cast<size_t>(2), batches.size());
    ASSERT_EQ(static_cast<size_t>(2), batches[0].size());
    ASSERT_EQ(static_cast<size_t>(1), batches[1].size());
}

int main() {
    return TestRunner::instance().runAll();
}
