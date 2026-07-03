cmake_minimum_required(VERSION 4.0)

project(Nadir
    LANGUAGES C
    VERSION 0.0.0
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Core Library ---
add_library(NadirCore STATIC)
add_library(Nadir::Core ALIAS NadirCore)

target_sources(NadirCore PRIVATE
    src/ast.c
    src/common.c
    src/compiler.c
    src/comptime.c
    src/lexer.c
    src/parser.c
    src/token.c
)

target_include_directories(NadirCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# --- Executable ---
add_executable(Nadir src/main.c)
target_link_libraries(Nadir PRIVATE Nadir::Core)
