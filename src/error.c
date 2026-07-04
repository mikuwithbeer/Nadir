#include "nadir/error.h"

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

    char *pointer = buffer;
    auto written = 0;
    auto remaining = NADIR_ERROR_STRING_MAXIMUM;

    switch (error->kind) {
        case NADIR_ERROR_KIND_NONE:
            break;

        case NADIR_ERROR_KIND_LEXER:
            written = snprintf(pointer,
                               remaining,
                               "%llu:%llu: error(lexer): ",
                               error->lexer.line,
                               error->lexer.column);

            remaining -= written;
            pointer += written;

            switch (error->lexer.kind) {
                case NADIR_LEXER_ERROR_KIND_NONE:
                    break;
                case NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW:
                    snprintf(pointer, remaining, "internal buffer overflow");
                    break;
                case NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "memory allocation failed");
                    break;
                case NADIR_LEXER_ERROR_KIND_UNKNOWN_CHARACTER:
                    snprintf(pointer,
                             remaining,
                             "unrecognized character '%c'",
                             error->lexer.specific.character);
                    break;
                case NADIR_LEXER_ERROR_KIND_NUMBER_TOO_LONG:
                    snprintf(pointer, remaining, "numeric literal is too long");
                    break;
                case NADIR_LEXER_ERROR_KIND_INVALID_NUMBER:
                    snprintf(pointer, remaining, "invalid numeric format");
                    break;
                case NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER:
                    snprintf(pointer,
                             remaining,
                             "unexpected character '%c'",
                             error->lexer.specific.character);
                    break;
                case NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE:
                    snprintf(pointer, remaining, "encountered an invalid state");
                    break;
            }

            break;

        case NADIR_ERROR_KIND_PARSER:
            if (error->parser.token == nullptr) {
                written = snprintf(pointer, remaining, "error(parser): ");
            } else {
                written = snprintf(pointer,
                                   remaining,
                                   "%llu:%llu: error(parser): ",
                                   error->parser.token->line,
                                   error->parser.token->column);
            }

            remaining -= written;
            pointer += written;

            switch (error->parser.kind) {
                case NADIR_PARSER_ERROR_KIND_NONE:
                    break;
                case NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "memory allocation failed");
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
                    snprintf(pointer, remaining, "block cannot be empty");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION:
                    snprintf(pointer, remaining, "expected an expression");
                    break;
                case NADIR_PARSER_ERROR_KIND_MISSING_SEMICOLON:
                    snprintf(pointer, remaining, "missing semicolon ';'");
                    break;
                case NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS:
                    snprintf(pointer, remaining, "too many arguments provided");
                    break;
                case NADIR_PARSER_ERROR_KIND_ALREADY_FOUND_BINARY:
                    snprintf(pointer, remaining, "binary definition already exists");
                    break;
            }

            break;

        case NADIR_ERROR_KIND_COMPILER:
            if (error->compiler.token == nullptr) {
                written = snprintf(pointer, remaining, "error(compiler): ");
            } else {
                written = snprintf(pointer,
                                   remaining,
                                   "%llu:%llu: error(compiler): ",
                                   error->compiler.token->line,
                                   error->compiler.token->column);
            }

            remaining -= written;
            pointer += written;

            switch (error->compiler.kind) {
                case NADIR_COMPILER_ERROR_KIND_NONE:
                    break;
                case NADIR_COMPILER_ERROR_KIND_EMPTY:
                    snprintf(pointer, remaining, "file is empty");
                    break;
                case NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY:
                    snprintf(pointer, remaining, "memory allocation failed");
                    break;
                case NADIR_COMPILER_ERROR_KIND_STACK_FAILED:
                    snprintf(pointer, remaining, "internal stack operation failed");
                    break;
                case NADIR_COMPILER_ERROR_KIND_COMPTIME_FAILED:
                    snprintf(pointer, remaining, "comptime evaluation failed for '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_CONSTANT:
                    snprintf(pointer,
                             remaining,
                             "duplicate declaration of constant '%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_PROCEDURE:
                    snprintf(pointer,
                             remaining,
                             "duplicate declaration of procedure '%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS:
                    snprintf(pointer,
                             remaining,
                             "duplicate declaration of address '%s'",
                             error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT:
                    snprintf(pointer, remaining, "undefined constant '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME:
                    snprintf(pointer, remaining, "undefined comptime function '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE:
                    snprintf(pointer, remaining, "undefined procedure '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS:
                    snprintf(pointer, remaining, "undefined address '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY:
                    snprintf(pointer, remaining, "undefined binary operation");
                    break;
                case NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH:
                    snprintf(pointer, remaining, "argument mismatch in call '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH:
                    snprintf(pointer, remaining, "type mismatch with value '%s'", error->compiler.token->value);
                    break;
                case NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH:
                    snprintf(pointer, remaining, "value out of byte range for '%s'", error->compiler.token->value);
                    break;
            }

            break;
    }

    return buffer;
}
