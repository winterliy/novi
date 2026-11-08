cmake_minimum_required(VERSION 3.16)
project(novi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NOVI_CORE_SOURCES
    src/novi/compiler.cpp
    src/novi/lexer.cpp
    src/novi/parser.cpp
    src/novi/project.cpp
    src/novi/resolver.cpp
    src/novi/runtime.cpp
    src/novi/vm.cpp
)
add_executable(novi src/main.cpp ${NOVI_CORE_SOURCES})
option(NOVI_BUILD_BENCHMARKS "Build benchmark executables" OFF)
if(NOVI_BUILD_BENCHMARKS)
    add_executable(novi_vm_benchmark benchmarks/vm_loop.cpp ${NOVI_CORE_SOURCES})
    target_include_directories(novi_vm_benchmark PRIVATE src)
endif()
target_include_directories(novi PRIVATE src)
target_compile_options(novi PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
    add_executable(novi_source_locations tests/source_locations.cpp)
    target_include_directories(novi_source_locations PRIVATE src)
    add_test(NAME novi_source_locations COMMAND novi_source_locations)
    add_test(
        NAME novi_runtime_regressions
        COMMAND ${CMAKE_COMMAND}
            "-DNOVI_EXECUTABLE=$<TARGET_FILE:novi>"
            "-DTEST_CASE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/cases"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_cases.cmake"
    )
endif()
