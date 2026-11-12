cmake_minimum_required(VERSION 3.14)
project(TokenizerProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(cpp/tokenization)

add_executable(tokenizer
    cpp/src/main.cpp
    cpp/src/Dataset.cpp
    cpp/src/Vocabulary.cpp
    cpp/src/IntegerEncoder.cpp
    cpp/src/OneHotEncoder.cpp
    cpp/src/BagOfWordsEncoder.cpp
    cpp/src/TfIdfEncoder.cpp
    cpp/src/StatisticsAnalyzer.cpp
)

target_include_directories(tokenizer PRIVATE cpp/include)
target_link_libraries(tokenizer PRIVATE tokenization)

if(MSVC)
    target_compile_options(tokenizer PRIVATE /utf-8)
endif()