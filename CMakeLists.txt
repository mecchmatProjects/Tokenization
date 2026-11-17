cmake_minimum_required(VERSION 3.14)
project(TokenizerProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(cmake/Dependencies.cmake)
add_subdirectory(cpp/tokenization)
add_subdirectory(cpp/preprocessing)
add_subdirectory(cpp/normalization)

set(TOKENIZER_CORE_SOURCES
    cpp/src/Dataset.cpp
    cpp/src/Vocabulary.cpp
    cpp/src/IntegerEncoder.cpp
    cpp/src/OneHotEncoder.cpp
    cpp/src/BagOfWordsEncoder.cpp
    cpp/src/TfIdfEncoder.cpp
    cpp/src/StatisticsAnalyzer.cpp
    cpp/src/Exporter.cpp
    cpp/src/Application.cpp
)

add_executable(tokenizer cpp/src/main.cpp cpp/src/MainWindow.cpp ${TOKENIZER_CORE_SOURCES})
target_include_directories(tokenizer PRIVATE cpp cpp/include)
target_link_libraries(tokenizer PRIVATE tokenization preprocessing normalization)

if(WIN32)
    target_link_libraries(tokenizer PRIVATE user32 gdi32 comctl32)
endif()

if(MSVC)
    target_compile_options(tokenizer PRIVATE /utf-8)
endif()

if(BUILD_TESTING)
    add_executable(application_preprocessing_tests
        cpp/preprocessing/tests/ApplicationPreprocessingTests.cpp
        cpp/src/MainWindow.cpp
        ${TOKENIZER_CORE_SOURCES}
    )
    target_include_directories(application_preprocessing_tests PRIVATE cpp cpp/include)
    target_link_libraries(application_preprocessing_tests PRIVATE tokenization preprocessing normalization)
    if(WIN32)
        target_link_libraries(application_preprocessing_tests PRIVATE user32 gdi32 comctl32)
    endif()
    if(MSVC)
        target_compile_options(application_preprocessing_tests PRIVATE /utf-8)
    endif()
    add_test(NAME application_preprocessing_tests COMMAND application_preprocessing_tests)
endif()
