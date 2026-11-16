cmake_minimum_required(VERSION 3.16)

# Project name and language (CXX means C++)
project(LitCritic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(litcritic
    src/sentence.cpp
    src/style_profile.cpp
    src/text_document.cpp
    src/token.cpp
)

target_include_directories(litcritic PUBLIC include)

add_executable(data_structures_tests tests/data_structures_tests.cpp)
target_link_libraries(data_structures_tests PRIVATE litcritic)

add_test(NAME data_structures_tests COMMAND data_structures_tests)