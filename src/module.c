/**
 * @file module.c
 * @brief The module implementation.
 */

#if defined(__linux__) || defined(__gnu_linux__)
#define _XOPEN_SOURCE 800
#elif defined(__APPLE__) && defined(__MACH__)
#define _DARWIN_C_SOURCE
#endif

#include "nadir/module.h"
#include "nadir/error.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

[[nodiscard]] static nadir_ast_t *nadir_module_parse(const nadir_module_t *module,
                                                     const char *absolute_path);

[[nodiscard]] static const char *nadir_module_path_last_seperator(const char *path);

[[nodiscard]] static char *nadir_module_path_absolute(nadir_arena_t *arena,
                                                      const char *path);

static void nadir_module_path_include(const char *current_file,
                                      const char *target_path,
                                      nadir_u64_t target_length,
                                      char *output);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_module_t *nadir_module_new(nadir_arena_t *arena) {
    nadir_module_t *module = nadir_arena_allocate(arena, sizeof(nadir_module_t));
    if (module == nullptr) {
        return nullptr;
    }

    auto const files = nadir_table_new(arena, sizeof(bool));
    if (files == nullptr) {
        return nullptr;
    }

    auto const ast = nadir_ast_new(arena);
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
    // Absolute paths ensure that the same module is not imported multiple times with different relative paths.
    char *absolute_path = nadir_module_path_absolute(module->arena, path);
    if (absolute_path == nullptr) {
        return false;
    }

    auto const absolute_length = strlen(absolute_path);
    auto const absolute_loaded = true;

    // Guard against importing the same module multiple times.
    if (!nadir_table_insert(module->files, absolute_path, absolute_length, &absolute_loaded)) {
        fprintf(stderr, "error(module): already imported '%s'\n", absolute_path);
        return false;
    }

    auto const ast = nadir_module_parse(module, absolute_path);
    if (ast == nullptr) {
        return false;
    }

    for (nadir_u64_t index = 0; index < ast->declarations->length; ++index) {
        const nadir_ast_declaration_t *declaration = nadir_list_get(ast->declarations, index);

        if (declaration->kind == NADIR_AST_DECLARATION_KIND_INCLUDE) {
            auto const path_start = declaration->include.path->string.value;
            auto const path_count = declaration->include.path->string.count;

            if (path_count >= NADIR_MODULE_PATH_MAXIMUM || path_count < NADIR_MODULE_PATH_MINIMUM) {
                fprintf(stderr,
                        "%s:%" PRIu32 ":%" PRIu32 ": error(module): invalid import path\n",
                        absolute_path,
                        declaration->include.path->line,
                        declaration->include.path->column);
                return false;
            }

            char next_path[NADIR_MODULE_PATH_MAXIMUM] = {}; // Next module path to resolve
            nadir_module_path_include(absolute_path, path_start, path_count, next_path);

            // Recursively resolve included modules.
            if (!nadir_module_resolve(module, next_path)) {
                return false;
            }
        } else {
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

static nadir_ast_t *nadir_module_parse(const nadir_module_t *module,
                                       const char *absolute_path) {
    auto error = (nadir_error_t){};

    auto const lexer = nadir_lexer_new(module->arena, absolute_path);
    if (lexer == nullptr) {
        fprintf(stderr, "error(lexer): failed to initialize for '%s'\n", absolute_path);
        return nullptr;
    }

    auto const lexer_error = nadir_lexer_collect(lexer);
    if (lexer_error.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_LEXER,
            .lexer = lexer_error
        };

        auto const message = nadir_error_encode(module->arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s\n", message);
        } else {
            fprintf(stderr, "error(lexer): out of memory\n");
        }

        return nullptr;
    }

    auto const parser = nadir_parser_new(module->arena, lexer->tokens);
    if (parser == nullptr) {
        fprintf(stderr, "error(parser): failed to initialize for '%s'\n", absolute_path);
        return nullptr;
    }

    auto const parser_error = nadir_parser_run(parser);
    if (parser_error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_PARSER,
            .parser = parser_error
        };

        auto const message = nadir_error_encode(module->arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s\n", message);
        } else {
            fprintf(stderr, "error(parser): out of memory\n");
        }

        return nullptr;
    }

    return parser->ast;
}

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
#if defined(_WIN32) || defined(_WIN64)
    char *absolute_path = nadir_arena_allocate(arena, NADIR_MODULE_PATH_MAXIMUM);
    if (absolute_path == nullptr) {
        fprintf(stderr, "error(module): out of memory\n");
        return nullptr;
    }

    if (_fullpath(absolute_path, path, NADIR_MODULE_PATH_MAXIMUM) == nullptr) {
        fprintf(stderr, "error(module): failed to resolve path for '%s'\n", path);
        return nullptr;
    }

    return absolute_path;
#else
    char *resolved_path = realpath(path, nullptr);
    if (resolved_path == nullptr) {
        fprintf(stderr, "error(module): failed to resolve path for '%s'\n", path);
        return nullptr;
    }

    auto const length = strlen(resolved_path) + 1;
    char *absolute_path = nadir_arena_allocate(arena, length);

    if (absolute_path == nullptr) {
        free(absolute_path);
        fprintf(stderr, "error(module): out of memory\n");
        return nullptr;
    }

    memcpy(absolute_path, resolved_path, length);
    free(resolved_path);

    return absolute_path;
#endif
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

    auto const directory_length = (int) (last_seperator - current_file + 1);
    snprintf(output,
             NADIR_MODULE_PATH_MAXIMUM,
             "%.*s%.*s",
             directory_length,
             current_file,
             (int) target_length,
             target_path);
}
