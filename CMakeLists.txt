cmake_minimum_required(VERSION 3.14)
project(TokenizerProject LANGUAGES C CXX)

add_subdirectory(cpp/tokenization)

add_executable(tokenizer cpp/src/main.cpp)
target_link_libraries(tokenizer PRIVATE tokenization)

if(MSVC)
    target_compile_options(tokenizer PRIVATE /utf-8)
endif()
