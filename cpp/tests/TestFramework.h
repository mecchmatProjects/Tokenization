#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <iomanip>

struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> fn;
};

class TestRunner {
private:
    std::vector<TestCase> tests;

public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void addTest(const std::string& suite, const std::string& name, std::function<void()> fn) {
        tests.push_back({suite, name, std::move(fn)});
    }

    int runAll() const {
        size_t passed = 0;
        size_t failed = 0;
        std::string currentSuite;

        std::cout << "====================================================================\n";
        std::cout << "                    NLP PIPELINE UNIT TEST SUITE                    \n";
        std::cout << "====================================================================\n";

        for (const auto& tc : tests) {
            if (tc.suite != currentSuite) {
                currentSuite = tc.suite;
                std::cout << "\n[Suite: " << currentSuite << "]\n";
            }

            try {
                tc.fn();
                ++passed;
                std::cout << "  [PASS] " << tc.name << "\n";
            } catch (const std::exception& ex) {
                ++failed;
                std::cout << "  [FAIL] " << tc.name << "\n"
                          << "         Reason: " << ex.what() << "\n";
            } catch (...) {
                ++failed;
                std::cout << "  [FAIL] " << tc.name << "\n"
                          << "         Reason: Unknown non-standard exception\n";
            }
        }

        std::cout << "\n====================================================================\n";
        std::cout << "Summary: " << passed << " passed, " << failed << " failed, "
                  << tests.size() << " total.\n";
        std::cout << "====================================================================\n";

        return (failed == 0) ? 0 : 1;
    }
};

struct TestRegistrar {
    TestRegistrar(const std::string& suite, const std::string& name, std::function<void()> fn) {
        TestRunner::instance().addTest(suite, name, std::move(fn));
    }
};

#define REGISTER_TEST(suite, name) \
    static void test_##suite##_##name(); \
    static TestRegistrar reg_##suite##_##name(#suite, #name, test_##suite##_##name); \
    static void test_##suite##_##name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            std::ostringstream oss; \
            oss << "Expected true at line " << __LINE__ << ": " #cond; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (false)

#define ASSERT_FALSE(cond) \
    do { \
        if ((cond)) { \
            std::ostringstream oss; \
            oss << "Expected false at line " << __LINE__ << ": " #cond; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (false)

#define ASSERT_EQ(expected, actual) \
    do { \
        auto expVal = (expected); \
        auto actVal = (actual); \
        if (!(expVal == actVal)) { \
            std::ostringstream oss; \
            oss << "Mismatch at line " << __LINE__ \
                << " | expected: [" << expVal << "], actual: [" << actVal << "]"; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (false)

#define ASSERT_NEAR(expected, actual, epsilon) \
    do { \
        double expVal = static_cast<double>(expected); \
        double actVal = static_cast<double>(actual); \
        if (std::fabs(expVal - actVal) > (epsilon)) { \
            std::ostringstream oss; \
            oss << std::fixed << std::setprecision(6) \
                << "Floating-point mismatch at line " << __LINE__ \
                << " | expected: " << expVal << ", actual: " << actVal \
                << " (eps=" << (epsilon) << ")"; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (false)