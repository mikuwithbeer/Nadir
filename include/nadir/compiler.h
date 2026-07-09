#ifndef NADIR_COMPILER_H
#define NADIR_COMPILER_H

/**
 * @file compiler.h
 * @brief The compiler interface.
 *
 * This file defines the compiler structure and related constants for
 * the assembler.
 */

#include "nadir/common/stack.h"
#include "nadir/common/table.h"
#include "nadir/ast.h"

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Error kinds for the compiler.
 */
typedef enum [[nodiscard]] : nadir_u8_t {
    NADIR_COMPILER_ERROR_KIND_NONE,
    NADIR_COMPILER_ERROR_KIND_EMPTY,
    NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY,
    NADIR_COMPILER_ERROR_KIND_STACK_FAILED,
    NADIR_COMPILER_ERROR_KIND_MULTIPLE_CONSTANT,
    NADIR_COMPILER_ERROR_KIND_MULTIPLE_PROCEDURE,
    NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY,
    NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH,
    NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH,
    NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH,

    NADIR_COMPILER_ERROR_KIND_COMPTIME_NULL_CONTEXT,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_ARGUMENT,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_SWAP_WIDTH,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_ASSERTION_FAILED,
} nadir_compiler_error_kind_t;

/**
 * @brief Error structure for the compiler.
 */
typedef struct [[nodiscard]] {
    nadir_compiler_error_kind_t kind;
    nadir_token_t *token;
} nadir_compiler_error_t;

/**
 * @brief Constant structure for the compiler.
 */
typedef struct {
    nadir_token_t *token;
    nadir_i128_t value;
} nadir_compiler_constant_t;

/**
 * @brief Procedure structure for the compiler.
 */
typedef struct {
    nadir_token_t *token;
    nadir_list_t *parameters; // List of `nadir_token_kind_t`
    nadir_list_t *statements; // List of `nadir_ast_expression_t`
} nadir_compiler_procedure_t;

/**
 * @brief Compiler structure for the assembler.
 */
typedef struct {
    nadir_arena_t *arena;
    nadir_arena_t *comptime_arena; // Arena for compile-time evaluation

    nadir_ast_t *ast;

    nadir_table_t *addresses; // Table of `nadir_u64_t`
    nadir_table_t *constants; // Table of `nadir_compiler_constant_t`
    nadir_table_t *procedures; // Table of `nadir_compiler_procedure_t`

    nadir_stack_t *stack;
    nadir_list_t *output; // List of `nadir_u8_t`

    nadir_u64_t binary_location; // Index of the binary declaration
    nadir_u64_t binary_origin; // Memory address origin for calculation
} nadir_compiler_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new compiler error with the given kind and token.
 */
static inline nadir_compiler_error_t nadir_compiler_error_new(const nadir_compiler_error_kind_t kind,
                                                              nadir_token_t *token) {
    return (nadir_compiler_error_t){
        .kind = kind,
        .token = token,
    };
}

/**
 * @brief Creates a new compiler with the given abstract syntax tree.
 */
[[nodiscard]] nadir_compiler_t *nadir_compiler_new(nadir_arena_t *arena,
                                                   nadir_arena_t *comptime_arena,
                                                   nadir_ast_t *ast);

/**
 * @brief Prepares the compiler by processing the abstract syntax tree and populating the necessary tables.
 */
nadir_compiler_error_t nadir_compiler_prepare(nadir_compiler_t *compiler);

/**
 * @brief Runs the compiler to generate the output binary based on the prepared tables and abstract syntax tree.
 */
nadir_compiler_error_t nadir_compiler_run(nadir_compiler_t *compiler);

/**
 * @brief Frees the memory allocated for the compiler.
 *
 * @warning The compiler is allocated on the arena and will be freed when the arena is freed.
 */
void nadir_compiler_free(nadir_compiler_t *compiler);

#endif //NADIR_COMPILER_H
