#ifndef NADIR_ERROR_H
#define NADIR_ERROR_H

/**
 * @file error.h
 * @brief The error interface.
 *
 * This file defines the error structure and related constants for
 * the assembler.
 */

#include "nadir/lexer.h"
#include "nadir/parser.h"
#include "nadir/compiler.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_ERROR_STRING_MAXIMUM = 1 << 9;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Error kinds for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_ERROR_KIND_NONE,
    NADIR_ERROR_KIND_LEXER,
    NADIR_ERROR_KIND_PARSER,
    NADIR_ERROR_KIND_COMPILER,
} nadir_error_kind_t;

/**
 * @brief Error structure for the assembler.
 */
typedef struct {
    nadir_error_kind_t kind;

    union {
        nadir_lexer_error_t lexer;
        nadir_parser_error_t parser;
        nadir_compiler_error_t compiler;
    };
} nadir_error_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Encodes the given error into a human-readable string.
 *
 * @warning The caller must free the return value.
 */
char *nadir_error_encode(const nadir_error_t *error);

#endif //NADIR_ERROR_H
