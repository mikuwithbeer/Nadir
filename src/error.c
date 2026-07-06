/**
 * @file error.c
 * @brief The error implementation.
 */

#include "nadir/error.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

char *nadir_error_encode(const nadir_error_t *error) {
    char *buffer = calloc(NADIR_ERROR_STRING_MAXIMUM, sizeof(char));
    if (buffer == nullptr) {
        return nullptr;
    }

    auto pointer = buffer;
    auto written = 0;
    auto remaining = (nadir_u64_t) NADIR_ERROR_STRING_MAXIMUM;

    switch (error->kind) {
        case NADIR_ERROR_KIND_NONE:
            break;

        case NADIR_ERROR_KIND_LEXER:
            written = snprintf(pointer,
                               remaining,
                               "%" PRIu64 ":%" PRIu64 ": error(lexer): ",
                               error->lexer.line,
                               error->lexer.column);

            remaining -= (nadir_u64_t) written;
            pointer += written;

            switch (error->lexer.kind) {
                case NADIR_LEXER_ERROR_KIND_NONE:
                    break;
                case NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW:
                    snprintf(pointer, remaining, "buffer overflow in tokenization");
                    break;
                case NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "memory allocation failed during tokenization");
                    break;
                case NADIR_LEXER_ERROR_KIND_UNKNOWN_CHARACTER:
                    snprintf(pointer, remaining, "unrecognized character '%c'", error->lexer.character);
                    break;
                case NADIR_LEXER_ERROR_KIND_NUMBER_TOO_LONG:
                    snprintf(pointer, remaining, "numeric literal exceeds maximum allowed length");
                    break;
                case NADIR_LEXER_ERROR_KIND_INVALID_NUMBER:
                    snprintf(pointer, remaining, "malformed numeric literal");
                    break;
                case NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER:
                    snprintf(pointer, remaining, "unexpected character '%c'", error->lexer.character);
                    break;
                case NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE:
                    snprintf(pointer, remaining, "reached an invalid internal state");
                    break;
            }
            break;

        case NADIR_ERROR_KIND_PARSER:
            if (error->parser.token == nullptr) {
                written = snprintf(pointer, remaining, " error(parser): ");
            } else {
                written = snprintf(pointer,
                                   remaining,
                                   "%" PRIu64 ":%" PRIu64 ": error(parser): ",
                                   error->parser.token->line,
                                   error->parser.token->column);
            }

            remaining -= (nadir_u64_t) written;
            pointer += written;

            switch (error->parser.kind) {
                case NADIR_PARSER_ERROR_KIND_NONE:
                    break;
                case NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "memory allocation failed during parsing");
                    break;
                case NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN:
                    snprintf(pointer, remaining, "unexpected token '%s'", error->parser.token->value);
                    break;
                case NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF:
                    snprintf(pointer, remaining, "unexpected end of file");
                    break;
                case NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION:
                    snprintf(pointer, remaining, "unexpected expression '%s'", error->parser.token->value);
                    break;
                case NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK:
                    snprintf(pointer, remaining, "code block cannot be empty");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION:
                    snprintf(pointer, remaining, "expected an expression");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_SEMICOLON:
                    snprintf(pointer, remaining, "expected ';' after statement");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_BINARY_ORIGIN:
                    snprintf(pointer, remaining, "missing origin in binary declaration");
                    break;
                case NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS:
                    snprintf(pointer, remaining, "too many arguments provided to procedure call");
                    break;
                case NADIR_PARSER_ERROR_KIND_ALREADY_FOUND_BINARY:
                    snprintf(pointer, remaining, "redefinition of binary declaration");
                    break;
                case NADIR_PARSER_ERROR_KIND_INVALID_BINARY_ORIGIN:
                    snprintf(pointer, remaining, "binary origin address is out of range");
                    break;
            }
            break;

        case NADIR_ERROR_KIND_COMPILER:
            if (error->compiler.token == nullptr) {
                written = snprintf(pointer, remaining, " error(compiler): ");
            } else {
                written = snprintf(pointer,
                                   remaining,
                                   "%" PRIu64 ":%" PRIu64 ": error(compiler): ",
                                   error->compiler.token->line,
                                   error->compiler.token->column);
            }

            remaining -= (nadir_u64_t) written;
            pointer += written;

            switch (error->compiler.kind) {
                case NADIR_COMPILER_ERROR_KIND_NONE:
                    break;
                case NADIR_COMPILER_ERROR_KIND_EMPTY:
                    snprintf(pointer, remaining, "input file contains no compilable code");
                    break;
                case NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "memory allocation failed during compilation");
                    break;
                case NADIR_COMPILER_ERROR_KIND_STACK_FAILED:
                    snprintf(pointer, remaining, "internal stack operation failed");
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_CONSTANT:
                    snprintf(pointer, remaining, "redefinition of constant '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_PROCEDURE:
                    snprintf(pointer, remaining, "redefinition of procedure '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS:
                    snprintf(pointer, remaining, "redefinition of address '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT:
                    snprintf(pointer, remaining, "use of undeclared constant '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME:
                    snprintf(pointer,
                             remaining,
                             "use of undefined comptime '@%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE:
                    snprintf(pointer, remaining, "use of undeclared procedure '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS:
                    snprintf(pointer, remaining, "use of undeclared address '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY:
                    snprintf(pointer, remaining, "binary declaration is missing");
                    break;
                case NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH:
                    snprintf(pointer,
                             remaining,
                             "argument count mismatch in call to '%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH:
                    snprintf(pointer, remaining, "incompatible type for value '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH:
                    snprintf(pointer, remaining, "value out of byte range for '%s'", error->compiler.token->value);
                    break;

                case NADIR_COMPILER_ERROR_KIND_COMPTIME_NULL_CONTEXT:
                    snprintf(pointer,
                             remaining,
                             "cannot evaluate comptime '@%s' without an execution context",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH:
                    snprintf(pointer,
                             remaining,
                             "incorrect number of arguments passed to comptime function '@%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND:
                    snprintf(pointer,
                             remaining,
                             "argument to comptime '@%s' evaluates out of bounds",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_ARGUMENT:
                    snprintf(pointer,
                             remaining,
                             "invalid argument value passed to comptime '@%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO:
                    snprintf(pointer,
                             remaining,
                             "division by zero during evaluation of comptime '@%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND:
                    snprintf(pointer,
                             remaining,
                             "bit shift operation in '@%s' exceeds the 0-127 bit range",
                             error->compiler.token->value);
                    break;
            }
            break;
    }

    return buffer;
}
