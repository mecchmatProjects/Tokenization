cmake_minimum_required(VERSION 3.14)
project(Tokenization LANGUAGES C CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Dependencies.cmake")

add_library(tokenization STATIC
    MockTokenizer.cpp
    TokenizerFactory.cpp
    TokenizerUtils.cpp
    WhitespaceTokenizer.cpp
    WordPunctuationTokenizer.cpp
)
target_compile_features(tokenization PUBLIC cxx_std_14)
target_include_directories(tokenization PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(tokenization PRIVATE utf8proc)

if(MSVC)
    target_compile_options(tokenization PRIVATE /utf-8)
endif()
