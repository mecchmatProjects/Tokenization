cmake_minimum_required(VERSION 3.14)
project(Tokenization LANGUAGES C CXX)

include(FetchContent)

set(UTF8PROC_INSTALL OFF CACHE BOOL "" FORCE)
set(UTF8PROC_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    utf8proc
    GIT_REPOSITORY https://github.com/JuliaStrings/utf8proc.git
    GIT_TAG v2.10.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(utf8proc)

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
