#ifndef NADIR_MODULE_H
#define NADIR_MODULE_H

/**
 * @file module.h
 * @brief The module interface.
 *
 * This file defines the module structure and related constants for
 * the assembler.
 */

#include "nadir/common/table.h"
#include "nadir/ast.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_MODULE_PATH_MINIMUM = 1 << 1;
constexpr auto NADIR_MODULE_PATH_MAXIMUM = 1 << 9;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Module structure for the assembler.
 */
typedef struct {
    nadir_arena_t *arena;
    nadir_table_t *files; // Table to track imported files
    nadir_ast_t *ast; // Summed abstract syntax tree
} nadir_module_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new module with the given arena.
 */
[[nodiscard]] nadir_module_t *nadir_module_new(nadir_arena_t *arena);

/**
 * @brief Resolves the given path and imports the module into the current module.
 */
[[nodiscard]] bool nadir_module_resolve(nadir_module_t *module,
                                        const char *path);

/**
 * @brief Frees the memory allocated for the module.
 *
 * @warning The module is allocated on the arena and will be freed when the arena is freed.
 */
void nadir_module_free(nadir_module_t *module);

#endif //NADIR_MODULE_H
