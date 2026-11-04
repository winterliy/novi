cmake_minimum_required(VERSION 3.16)
project(novi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(novi
    src/main.cpp
    src/novi/compiler.cpp
    src/novi/lexer.cpp
    src/novi/parser.cpp
    src/novi/project.cpp
    src/novi/runtime.cpp
    src/novi/vm.cpp
)
target_include_directories(novi PRIVATE src)
target_compile_options(novi PRIVATE -Wall -Wextra -Wpedantic)
