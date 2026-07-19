cmake_minimum_required(VERSION 4.0)

project(
    Nadir
    LANGUAGES C
    VERSION 0.2.1
)

# --- Language Standards ---
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Compilation Options ---
add_library(Options INTERFACE)
add_library(Nadir::Options ALIAS Options)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

target_compile_options(
    Options INTERFACE
    # --- Warnings ---
    -Wall
    -Wextra
    -Wpedantic
    # --- Type Safety ---
    -Wsign-conversion
    -Wconversion
    -Wshadow
    # --- Function Declarations ---
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wold-style-definition
    -Werror=implicit-function-declaration
    # --- Format Strings ---
    -Wformat=2
    # --- Code Quality ---
    -Wundef
    -Wnull-dereference
    -Wpointer-arith
    -Wcast-align
    -Wcast-qual
    -Wbad-function-cast
    -Wvla
    -Wimplicit-fallthrough
    -Wunused-macros
    # --- Release ---
    $<$<CONFIG:Release>:-O3>
)

target_compile_definitions(
    Options INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# --- Core Library ---
add_library(Core STATIC)
add_library(Nadir::Core ALIAS Core)

target_sources(
    Core PRIVATE
    src/common/arena.c
    src/common/list.c
    src/common/number.c
    src/common/stack.c
    src/common/table.c
    src/ast.c
    src/cli.c
    src/compiler.c
    src/comptime.c
    src/error.c
    src/lexer.c
    src/module.c
    src/parser.c
    src/token.c
)

target_include_directories(
    Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(
    Core PRIVATE
    NADIR_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(Core PRIVATE Nadir::Options)

# --- Executable ---
add_executable(Nadir src/main.c)
target_link_libraries(Nadir PRIVATE Nadir::Core)

# --- Testing ---
enable_testing()
add_subdirectory(test)
