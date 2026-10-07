cmake_minimum_required(VERSION 3.16)
project(TreeWalkInterpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(treewalk
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/environment.cpp
    src/interpreter.cpp
    src/tracer.cpp
    src/token_names.cpp
)

target_include_directories(treewalk PRIVATE include)

if (MSVC)
    target_compile_options(treewalk PRIVATE /W4)
else()
    target_compile_options(treewalk PRIVATE -Wall -Wextra -pedantic)
endif()
