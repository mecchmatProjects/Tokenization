cmake_minimum_required(VERSION 3.16)

project(LitCritic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(litcritic
    src/lexical_analyzer.cpp
 "include/sentence_analyzer.h" "src/sentence_analyzer.cpp")

target_include_directories(litcritic PUBLIC include)

add_executable(lexical_analyzer_tests 
    tests/lexical_analyzer_tests.cpp
 "include/sentence_analyzer.h" "src/sentence_analyzer.cpp")

target_link_libraries(lexical_analyzer_tests PRIVATE litcritic)

add_test(NAME LexicalAnalyzerTests COMMAND lexical_analyzer_tests)
