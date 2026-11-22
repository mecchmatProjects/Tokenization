cmake_minimum_required(VERSION 3.16)

# Project name and language (CXX means C++)
project(LitCritic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(litcritic
    src/lemmatizer.cpp
    src/lexical_analyzer.cpp
    src/preprocessor.cpp
    src/sentence.cpp
    src/sentence_analyzer.cpp
    src/style_profile.cpp
    src/text_document.cpp
    src/text_loader.cpp
    src/token.cpp
    src/tokenizer.cpp
 "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")

target_include_directories(litcritic PUBLIC include)

# Tests
add_executable(lexical_analyzer_tests tests/lexical_analyzer_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(lexical_analyzer_tests PRIVATE litcritic)
add_test(NAME LexicalAnalyzerTests COMMAND lexical_analyzer_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(data_structures_tests tests/data_structures_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(data_structures_tests PRIVATE litcritic)
add_test(NAME data_structures_tests COMMAND data_structures_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(text_loader_tests tests/text_loader_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(text_loader_tests PRIVATE litcritic)
add_test(NAME text_loader_tests COMMAND text_loader_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(preprocessor_tests tests/preprocessor_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(preprocessor_tests PRIVATE litcritic)
add_test(NAME preprocessor_tests COMMAND preprocessor_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(sentence_analyzer_tests tests/sentence_analyzer_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(sentence_analyzer_tests PRIVATE litcritic)
add_test(NAME sentence_analyzer_tests COMMAND sentence_analyzer_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(tokenizer_tests tests/tokenizer_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(tokenizer_tests PRIVATE litcritic)
add_test(NAME tokenizer_tests COMMAND tokenizer_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(lemmatizer_tests tests/lemmatizer_tests.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(lemmatizer_tests PRIVATE litcritic)
add_test(NAME lemmatizer_tests COMMAND lemmatizer_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_pipeline tests/test_pipeline.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(test_pipeline PRIVATE litcritic)
add_test(NAME PipelineTests COMMAND test_pipeline WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})



# Main application
add_executable(litcritic_app main.cpp "include/text_comparator.h" "src/text_comparator.cpp" "tests/text_comparator_tests.cpp")
target_link_libraries(litcritic_app PRIVATE litcritic)
