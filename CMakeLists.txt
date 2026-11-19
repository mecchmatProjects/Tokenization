cmake_minimum_required(VERSION 3.16)

# Project name and language (CXX means C++)
project(LitCritic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(litcritic
)

target_include_directories(litcritic PUBLIC include)



