cmake_minimum_required(VERSION 3.14)
project(Normalization LANGUAGES C CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Dependencies.cmake")
if(NOT TARGET tokenization)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../tokenization" "${CMAKE_CURRENT_BINARY_DIR}/tokenization")
endif()

add_library(normalization STATIC
    Normalizer.cpp
    StopWordDictionary.cpp
    UnicodeUtils.cpp
)
target_compile_features(normalization PUBLIC cxx_std_14)
target_include_directories(normalization PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(normalization PRIVATE utf8proc tokenization)

if(MSVC)
    target_compile_options(normalization PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(normalizer_tests tests/NormalizerTests.cpp)
    target_link_libraries(normalizer_tests PRIVATE normalization tokenization)
    if(MSVC)
        target_compile_options(normalizer_tests PRIVATE /utf-8)
    endif()
    add_test(NAME normalizer_tests COMMAND normalizer_tests)
endif()
