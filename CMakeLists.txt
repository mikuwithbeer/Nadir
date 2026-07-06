cmake_minimum_required(VERSION 4.0)

project(Nadir
    LANGUAGES C
    VERSION 0.1.0
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Compilation Options ---
add_library(Options INTERFACE)
add_library(Nadir::Options ALIAS Options)

target_compile_options(Options INTERFACE
    # --- Warnings ---
    -Wall
    -Wextra
    -Wpedantic
    # --- Type Safety ---
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
)

target_compile_definitions(Options INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
)

# --- Core Library ---
add_library(Core STATIC)
add_library(Nadir::Core ALIAS Core)

target_sources(Core PRIVATE
    src/ast.c
    src/cli.c
    src/common.c
    src/compiler.c
    src/comptime.c
    src/error.c
    src/lexer.c
    src/parser.c
    src/token.c
)

target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(Core PRIVATE
    NADIR_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(Core PRIVATE Nadir::Options)

# --- Executable ---
add_executable(Nadir src/main.c)
target_link_libraries(Nadir PRIVATE Nadir::Core)

# --- Testing ---
enable_testing()
add_subdirectory(test)
