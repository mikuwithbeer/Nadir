/**
 * @file module.c
 * @brief The module implementation.
 */

#define _POSIX_C_SOURCE 202405L

#include "nadir/module.h"
#include "nadir/error.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

[[nodiscard]] static const char *nadir_module_path_last_seperator(const char *path);

[[nodiscard]] static char *nadir_module_path_absolute(nadir_arena_t *arena,
                                                      const char *path);

static void nadir_module_path_include(const char *current_file,
                                      const char *target_path,
                                      nadir_u64_t target_length,
                                      char *output);

[[nodiscard]] static nadir_ast_t *nadir_module_parse(const nadir_module_t *module,
                                                     const char *absolute_path);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_module_t *nadir_module_new(nadir_arena_t *arena) {
    nadir_module_t *module = nadir_arena_allocate(arena, sizeof(nadir_module_t));
    if (module == nullptr) {
        return nullptr;
    }

    const auto files = nadir_table_new(arena, sizeof(bool));
    if (files == nullptr) {
        return nullptr;
    }

    const auto ast = nadir_ast_new(arena);
    if (ast == nullptr) {
        return nullptr;
    }

    module->arena = arena;
    module->files = files;
    module->ast = ast;

    return module;
}

bool nadir_module_resolve(nadir_module_t *module,
                          const char *path) {
    // Resolve the absolute path of the module to be imported.
    char *absolute_path = nadir_module_path_absolute(module->arena, path);
    if (absolute_path == nullptr) {
        return false;
    }

    const auto absolute_length = strlen(absolute_path);
    const auto absolute_loaded = true;

    // Guard against importing the same module multiple times.
    if (!nadir_table_insert(module->files, absolute_path, absolute_length, &absolute_loaded)) {
        fprintf(stderr, "error(module): already imported '%s'\n", absolute_path);
        return false;
    }

    // Parse the module and collect its abstract syntax tree.
    const auto ast = nadir_module_parse(module, absolute_path);
    if (ast == nullptr) {
        return false;
    }

    for (nadir_u64_t index = 0; index < ast->declarations->length; ++index) {
        const nadir_ast_declaration_t *declaration = nadir_list_get(ast->declarations, index);

        // Handle include declarations by resolving the included module.
        if (declaration->kind == NADIR_AST_DECLARATION_KIND_INCLUDE) {
            const auto path_start = declaration->include.path->string.value;
            const auto path_count = declaration->include.path->string.count;

            if (path_count >= NADIR_MODULE_PATH_MAXIMUM || path_count < NADIR_MODULE_PATH_MINIMUM) {
                fprintf(stderr,
                        "%s:%" PRIu32 ":%" PRIu32 ": error(module): invalid import path\n",
                        absolute_path,
                        declaration->include.path->line,
                        declaration->include.path->column);
                return false;
            }

            char next_path[NADIR_MODULE_PATH_MAXIMUM] = {};
            nadir_module_path_include(absolute_path, path_start, path_count, next_path);

            // Resolve the included module recursively.
            if (!nadir_module_resolve(module, next_path)) {
                return false;
            }
        } else {
            // Append the declaration to the current abstract syntax tree.
            if (!nadir_list_append(module->ast->declarations, declaration)) {
                fprintf(stderr, "error(ast): out of memory\n");
                return false;
            }
        }
    }

    return true;
}

void nadir_module_free(nadir_module_t *module) {
    if (module == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    module->files = nullptr;
    module->ast = nullptr;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static const char *nadir_module_path_last_seperator(const char *path) {
    const char *last_slash = strrchr(path, '/');

#if defined(_WIN32) || defined(_WIN64)
    const char *last_backslash = strrchr(path, '\\');
    if (last_backslash > last_slash) {
        return last_backslash;
    }
#endif

    return last_slash;
}

static char *nadir_module_path_absolute(nadir_arena_t *arena,
                                        const char *path) {
    char *absolute_path = nadir_arena_allocate(arena, NADIR_MODULE_PATH_MAXIMUM * sizeof(char));
    if (absolute_path == nullptr) {
        fprintf(stderr, "error(module): out of memory\n");
        return nullptr;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (_fullpath(absolute_path, path, NADIR_MODULE_PATH_MAXIMUM) == nullptr) {
#else
    if (realpath(path, absolute_path) == nullptr) {
#endif
        fprintf(stderr, "error(module): failed to resolve path for '%s'\n", path);
        return nullptr;
    }

    return absolute_path;
}

static void nadir_module_path_include(const char *current_file,
                                      const char *target_path,
                                      const nadir_u64_t target_length,
                                      char *output) {
    const char *last_seperator = nadir_module_path_last_seperator(current_file);
    if (last_seperator == nullptr) {
        snprintf(output, NADIR_MODULE_PATH_MAXIMUM, "%.*s", (int) target_length, target_path);
        return;
    }

    const auto directory_length = (int) (last_seperator - current_file + 1);
    snprintf(output,
             NADIR_MODULE_PATH_MAXIMUM,
             "%.*s%.*s",
             directory_length,
             current_file,
             (int) target_length,
             target_path);
}

static nadir_ast_t *nadir_module_parse(const nadir_module_t *module,
                                       const char *absolute_path) {
    auto error = (nadir_error_t){};

    // Initialize the lexer for the given module path.
    const auto lexer = nadir_lexer_new(module->arena, absolute_path);
    if (lexer == nullptr) {
        fprintf(stderr, "error(lexer): failed to initialize for '%s'\n", absolute_path);
        return nullptr;
    }

    // Run the lexer to collect tokens from the source.
    const auto lexer_error = nadir_lexer_collect(lexer);
    if (lexer_error.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_LEXER,
            .lexer = lexer_error
        };

        const auto message = nadir_error_encode(module->arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s\n", message);
        } else {
            fprintf(stderr, "error(lexer): out of memory\n");
        }

        return nullptr;
    }

    // Initialize the parser for the collected tokens.
    const auto parser = nadir_parser_new(module->arena, lexer->tokens);
    if (parser == nullptr) {
        fprintf(stderr, "error(parser): failed to initialize for '%s'\n", absolute_path);
        return nullptr;
    }

    // Run the parser to generate the abstract syntax tree.
    const auto parser_error = nadir_parser_run(parser);
    if (parser_error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_PARSER,
            .parser = parser_error
        };

        const auto message = nadir_error_encode(module->arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s\n", message);
        } else {
            fprintf(stderr, "error(parser): out of memory\n");
        }

        return nullptr;
    }

    return parser->ast;
}
