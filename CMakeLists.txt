cmake_minimum_required(VERSION 3.16)
project(novi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NOVI_ASCII_BACKEND "cpp" CACHE STRING "ASCII prefix backend: cpp, rust, or asm")
set_property(CACHE NOVI_ASCII_BACKEND PROPERTY STRINGS cpp rust asm)
if(NOVI_ASCII_BACKEND STREQUAL "cpp")
    add_library(novi_ascii STATIC src/native/ascii.cpp)
elseif(NOVI_ASCII_BACKEND STREQUAL "asm")
    if(NOT APPLE OR NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
        message(FATAL_ERROR "ASM backend requires macOS on arm64")
    endif()
    enable_language(ASM)
    add_library(novi_ascii STATIC src/native/ascii_arm64_macos.s)
    set_target_properties(novi_ascii PROPERTIES LINKER_LANGUAGE CXX)
elseif(NOVI_ASCII_BACKEND STREQUAL "rust")
    find_program(RUSTC rustc REQUIRED)
    set(RUST_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/libnovi_ascii_rust.a")
    add_custom_command(OUTPUT "${RUST_LIBRARY}"
        COMMAND "${RUSTC}" --crate-type staticlib -C opt-level=3 --edition=2021
                "${CMAKE_CURRENT_SOURCE_DIR}/src/native/ascii.rs" -o "${RUST_LIBRARY}"
        DEPENDS src/native/ascii.rs VERBATIM)
    add_custom_target(novi_ascii_build DEPENDS "${RUST_LIBRARY}")
    add_library(novi_ascii STATIC IMPORTED GLOBAL)
    set_target_properties(novi_ascii PROPERTIES IMPORTED_LOCATION "${RUST_LIBRARY}")
    add_dependencies(novi_ascii novi_ascii_build)
else()
    message(FATAL_ERROR "Unknown NOVI_ASCII_BACKEND: ${NOVI_ASCII_BACKEND}")
endif()

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
target_link_libraries(novi PRIVATE novi_ascii)
option(NOVI_BUILD_BENCHMARKS "Build benchmark executables" OFF)
if(NOVI_BUILD_BENCHMARKS)
    add_executable(novi_ascii_benchmark benchmarks/ascii_prefix.cpp)
    target_include_directories(novi_ascii_benchmark PRIVATE src)
    target_link_libraries(novi_ascii_benchmark PRIVATE novi_ascii)
    add_executable(novi_vm_benchmark benchmarks/vm_loop.cpp ${NOVI_CORE_SOURCES})
    target_include_directories(novi_vm_benchmark PRIVATE src)
    target_link_libraries(novi_vm_benchmark PRIVATE novi_ascii)
endif()
target_include_directories(novi PRIVATE src)
target_compile_options(novi PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
    add_executable(novi_ascii_test tests/ascii_prefix.cpp)
    target_include_directories(novi_ascii_test PRIVATE src)
    target_link_libraries(novi_ascii_test PRIVATE novi_ascii)
    add_test(NAME novi_ascii_test COMMAND novi_ascii_test)
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
