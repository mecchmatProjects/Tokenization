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
    cpp/src/Word.cpp
    cpp/src/Application.cpp
)

add_executable(tokenizer cpp/src/main.cpp cpp/src/MainWindow.cpp ${TOKENIZER_CORE_SOURCES})
target_include_directories(tokenizer PRIVATE cpp cpp/include)
target_link_libraries(tokenizer PRIVATE tokenization preprocessing normalization)

if(WIN32)
    target_link_libraries(tokenizer PRIVATE user32 gdi32 comctl32 ole32 uuid)
endif()

if(MSVC)
    target_compile_options(tokenizer PRIVATE /utf-8)
endif()

if(BUILD_TESTING)
    add_executable(text_loading_tests
        cpp/preprocessing/tests/TextLoadingTests.cpp
        cpp/src/MainWindow.cpp
        ${TOKENIZER_CORE_SOURCES}
    )
    target_include_directories(text_loading_tests PRIVATE cpp cpp/include)
    target_link_libraries(text_loading_tests PRIVATE tokenization preprocessing normalization)
    if(WIN32)
        target_link_libraries(text_loading_tests PRIVATE user32 gdi32 comctl32 ole32 uuid)
    endif()
    if(MSVC)
        target_compile_options(text_loading_tests PRIVATE /utf-8)
    endif()
    add_test(NAME text_loading_tests COMMAND text_loading_tests)

    add_executable(application_preprocessing_tests
        cpp/preprocessing/tests/ApplicationPreprocessingTests.cpp
        cpp/src/MainWindow.cpp
        ${TOKENIZER_CORE_SOURCES}
    )
    target_include_directories(application_preprocessing_tests PRIVATE cpp cpp/include)
    target_link_libraries(application_preprocessing_tests PRIVATE tokenization preprocessing normalization)
    if(WIN32)
        target_link_libraries(application_preprocessing_tests PRIVATE user32 gdi32 comctl32 ole32 uuid)
    endif()
    if(MSVC)
        target_compile_options(application_preprocessing_tests PRIVATE /utf-8)
    endif()
    add_test(NAME application_preprocessing_tests COMMAND application_preprocessing_tests)

    add_executable(unit_tests
        cpp/tests/test_main.cpp
        ${TOKENIZER_CORE_SOURCES}
    )
    target_include_directories(unit_tests PRIVATE cpp cpp/include cpp/tests)
    target_link_libraries(unit_tests PRIVATE tokenization preprocessing normalization)

    add_executable(morphology_tests tests/test_morphology.cpp cpp/src/Word.cpp)
    target_include_directories(morphology_tests PRIVATE cpp/include)

    if(MSVC)
        target_compile_options(unit_tests PRIVATE /utf-8)
        target_compile_options(morphology_tests PRIVATE /utf-8)
    endif()
    add_test(NAME PipelineUnitTests COMMAND unit_tests)
    add_test(NAME morphology_tests COMMAND morphology_tests)
endif()
