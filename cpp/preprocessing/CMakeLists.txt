cmake_minimum_required(VERSION 3.14)
project(Preprocessing LANGUAGES C CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Dependencies.cmake")

add_library(preprocessing STATIC TextPreprocessor.cpp)
target_compile_features(preprocessing PUBLIC cxx_std_17)
target_include_directories(preprocessing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(preprocessing PRIVATE utf8proc)

if(MSVC)
    target_compile_options(preprocessing PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(preprocessing_tests tests/TextPreprocessorTests.cpp)
    target_link_libraries(preprocessing_tests PRIVATE preprocessing)
    if(MSVC)
        target_compile_options(preprocessing_tests PRIVATE /utf-8)
    endif()
    add_test(NAME preprocessing_tests COMMAND preprocessing_tests)
endif()
