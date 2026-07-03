#ifndef NADIR_COMPILER_H
#define NADIR_COMPILER_H

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
    NADIR_COMPILER_ERROR_KIND_TABLE_FAILED,
    NADIR_COMPILER_ERROR_KIND_STACK_FAILED,
    NADIR_COMPILER_ERROR_KIND_COMPTIME_FAILED,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS,
    NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY,
    NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH,
    NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH,
    NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH,
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

    nadir_list_t *parameters;
    nadir_list_t *statements;
} nadir_compiler_procedure_t;

/**
 * @brief Compiler structure for the compiler.
 */
typedef struct {
    nadir_ast_t *ast;

    nadir_table_t *constants;
    nadir_table_t *addresses;
    nadir_table_t *procedures;

    nadir_list_t *output;

    nadir_u64_t expected;
    nadir_u64_t location;

    nadir_stack_t stack;
} nadir_compiler_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new analyzer error with the given parameters.
 */
static inline nadir_compiler_error_t nadir_compiler_error_new(const nadir_compiler_error_kind_t kind,
                                                              nadir_token_t *token) {
    return (nadir_compiler_error_t){
        .kind = kind,
        .token = token,
    };
}

/**
 * @brief Creates a new compiler structure with the given abstract syntax tree.
 */
[[nodiscard]] nadir_compiler_t *nadir_compiler_new(nadir_ast_t *ast);

/**
 * @brief Prepares the compiler by evaluating constants and procedures in the abstract syntax tree.
 */
nadir_compiler_error_t nadir_compiler_prepare(nadir_compiler_t *compiler);

/**
 * @brief Runs the compiler on the given abstract syntax tree.
 */
nadir_compiler_error_t nadir_compiler_run(nadir_compiler_t *compiler);

/**
 * @brief Frees the memory allocated for the compiler and its associated resources.
 */
void nadir_compiler_free(nadir_compiler_t *compiler);

#endif //NADIR_COMPILER_H
