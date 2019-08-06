cmake_minimum_required(VERSION 3.5)
project(PDJsonParser
    DESCRIPTION "a tiny JsonParser"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(PDJson SHARED
  pdjson.hpp
  pdjson.cc
)

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options(PDJson PRIVATE -Wall)
endif()

