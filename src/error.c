/**
 * @file error.c
 * @brief The error implementation.
 */

#include "nadir/error.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

char *nadir_error_encode(nadir_arena_t *arena,
                         const nadir_error_t *error) {
    char *buffer = nadir_arena_allocate(arena, NADIR_ERROR_STRING_MAXIMUM * sizeof(char));
    if (buffer == nullptr) {
        return nullptr;
    }

    // Clear the buffer before writing.
    memset(buffer, 0, NADIR_ERROR_STRING_MAXIMUM * sizeof(char));

    auto pointer = buffer;
    auto written = 0;
    auto remaining = (nadir_u64_t) NADIR_ERROR_STRING_MAXIMUM;

    switch (error->kind) {
        case NADIR_ERROR_KIND_NONE:
            break;

        case NADIR_ERROR_KIND_LEXER:
            written = snprintf(pointer,
                               remaining,
                               "%s:%" PRIu32 ":%" PRIu32 ": error(lexer): ",
                               error->lexer.path,
                               error->lexer.line,
                               error->lexer.column);

            remaining -= (nadir_u64_t) written;
            pointer += written;

            switch (error->lexer.kind) {
                case NADIR_LEXER_ERROR_KIND_NONE:
                    break;
                case NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW:
                    snprintf(pointer, remaining, "token is too long");
                    break;
                case NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "failed to allocate memory while tokenizing");
                    break;
                case NADIR_LEXER_ERROR_KIND_ILLEGAL_CHARACTER:
                    snprintf(pointer, remaining, "unrecognized character '%c'", error->lexer.character);
                    break;
                case NADIR_LEXER_ERROR_KIND_INVALID_NUMBER:
                    snprintf(pointer, remaining, "invalid numeric literal");
                    break;
                case NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER:
                    snprintf(pointer, remaining, "unexpected character '%c'", error->lexer.character);
                    break;
                case NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE:
                    snprintf(pointer, remaining, "oops, the lexer reached an unexpected internal state");
                    break;
            }
            break;

        case NADIR_ERROR_KIND_PARSER:
            if (error->parser.token == nullptr) {
                written = snprintf(pointer, remaining, " error(parser): ");
            } else {
                written = snprintf(pointer,
                                   remaining,
                                   "%s:%" PRIu32 ":%" PRIu32 ": error(parser): ",
                                   error->parser.token->path,
                                   error->parser.token->line,
                                   error->parser.token->column);
            }

            remaining -= (nadir_u64_t) written;
            pointer += written;

            switch (error->parser.kind) {
                case NADIR_PARSER_ERROR_KIND_NONE:
                    break;
                case NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "failed to allocate memory while parsing");
                    break;
                case NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN:
                    snprintf(pointer, remaining, "expected token '%s', found '%s'",
                             nadir_token_kind_encode(error->parser.expected),
                             nadir_token_kind_encode(error->parser.token->kind));
                    break;
                case NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF:
                    snprintf(pointer, remaining, "unexpected end of file");
                    break;
                case NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION:
                    snprintf(pointer, remaining, "this expression is not valid here");
                    break;
                case NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK:
                    snprintf(pointer, remaining, "this block cannot be empty");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION:
                    snprintf(pointer, remaining, "expected an expression");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_BINARY_ORIGIN:
                    snprintf(pointer, remaining, "binary declaration requires an origin");
                    break;
                case NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS:
                    snprintf(pointer, remaining, "too many arguments in procedure call");
                    break;
                case NADIR_PARSER_ERROR_KIND_ALREADY_FOUND_BINARY:
                    snprintf(pointer, remaining, "binary has already been declared");
                    break;
                case NADIR_PARSER_ERROR_KIND_INVALID_BINARY_ORIGIN:
                    snprintf(pointer, remaining, "binary origin is out of range");
                    break;
            }
            break;

        case NADIR_ERROR_KIND_COMPILER:
            if (error->compiler.token == nullptr) {
                written = snprintf(pointer, remaining, " error(compiler): ");
            } else {
                written = snprintf(pointer,
                                   remaining,
                                   "%s:%" PRIu32 ":%" PRIu32 ": error(compiler): ",
                                   error->compiler.token->path,
                                   error->compiler.token->line,
                                   error->compiler.token->column);
            }

            remaining -= (nadir_u64_t) written;
            pointer += written;

            switch (error->compiler.kind) {
                case NADIR_COMPILER_ERROR_KIND_NONE:
                    break;
                case NADIR_COMPILER_ERROR_KIND_EMPTY:
                    snprintf(pointer, remaining, "nothing to compile");
                    break;
                case NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "failed to allocate memory during compilation");
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_CONSTANT:
                    snprintf(pointer,
                             remaining,
                             "constant '%.*s' is already defined",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_PROCEDURE:
                    snprintf(pointer,
                             remaining,
                             "procedure '%.*s' is already defined",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS:
                    snprintf(pointer,
                             remaining,
                             "address '%.*s' is already defined",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT:
                    snprintf(pointer,
                             remaining,
                             "unknown constant '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME:
                    snprintf(pointer,
                             remaining,
                             "unknown comptime function '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE:
                    snprintf(pointer,
                             remaining,
                             "unknown procedure '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS:
                    snprintf(pointer,
                             remaining,
                             "unknown address '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY:
                    snprintf(pointer, remaining, "missing binary declaration");
                    break;
                case NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH:
                    snprintf(pointer,
                             remaining,
                             "wrong number of arguments for '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_VALUE_OUT_OF_BOUNDS:
                    snprintf(pointer,
                             remaining,
                             "expected a value of type %s",
                             nadir_token_kind_encode(error->compiler.expected));
                    break;
                case NADIR_COMPILER_ERROR_KIND_PADDING_OUT_OF_RANGE:
                    snprintf(pointer, remaining, "padding value is out of range");
                    break;
                case NADIR_COMPILER_ERROR_KIND_ALREADY_FOUND_BINARY:
                    snprintf(pointer, remaining, "binary has already been declared");
                    break;

                case NADIR_COMPILER_ERROR_KIND_COMPTIME_NULL_CONTEXT:
                    snprintf(pointer,
                             remaining,
                             "cannot evaluate '%.*s' in this context",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH:
                    snprintf(pointer,
                             remaining,
                             "'%.*s' was called with the wrong number of arguments",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND:
                    snprintf(pointer,
                             remaining,
                             "an argument to '%.*s' is out of range",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_ARGUMENT:
                    snprintf(pointer,
                             remaining,
                             "invalid argument for '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO:
                    snprintf(pointer,
                             remaining,
                             "division by zero while evaluating '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND:
                    snprintf(pointer,
                             remaining,
                             "shift amount must be between 0 and 127 while evaluating '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH:
                    snprintf(pointer,
                             remaining,
                             "invalid bit width for '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_ASSERTION_FAILED:
                    snprintf(pointer,
                             remaining,
                             "assertion failed while evaluating '%.*s'",
                             (int) error->compiler.token->string.count,
                             error->compiler.token->string.value);
                    break;
            }
            break;
    }

    return buffer;
}
