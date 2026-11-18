cmake_minimum_required(VERSION 3.14)
project(TokenizerProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(cpp/tokenization)

set(CORE_SOURCES
    cpp/src/Dataset.cpp
    cpp/src/Vocabulary.cpp
    cpp/src/IntegerEncoder.cpp
    cpp/src/OneHotEncoder.cpp
    cpp/src/BagOfWordsEncoder.cpp
    cpp/src/TfIdfEncoder.cpp
    cpp/src/StatisticsAnalyzer.cpp
    cpp/src/Exporter.cpp
    cpp/src/MainWindow.cpp
    feature/morphology-word
    cpp/src/Word.cpp
    cpp/src/Application.cpp
    develop_cpp
    tests/test_morphology.cpp
)

add_executable(tokenizer
    cpp/src/main.cpp
    cpp/src/MainWindow.cpp
    ${CORE_SOURCES}
)

target_include_directories(tokenizer PRIVATE cpp cpp/include)
target_link_libraries(tokenizer PRIVATE tokenization)

if(WIN32)
    target_link_libraries(tokenizer PRIVATE user32 gdi32 comctl32)
endif()

if(MSVC)
    target_compile_options(tokenizer PRIVATE /utf-8)
endif()

# Unit test suite target
add_executable(unit_tests
    cpp/tests/test_main.cpp
    ${CORE_SOURCES}
)

target_include_directories(unit_tests PRIVATE cpp cpp/include cpp/tests)
target_link_libraries(unit_tests PRIVATE tokenization)

if(MSVC)
    target_compile_options(unit_tests PRIVATE /utf-8)
endif()

enable_testing()
add_test(NAME PipelineUnitTests COMMAND unit_tests)