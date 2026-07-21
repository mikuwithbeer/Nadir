/**
 * @file token.c
 * @brief The token implementation.
 */

#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_token_t nadir_token_new(const char *path,
                              const nadir_token_kind_t kind,
                              const nadir_u32_t line,
                              const nadir_u32_t column) {
    return (nadir_token_t){
        .path = path,
        .kind = kind,

        .column = column,
        .line = line,
    };
}

void nadir_token_start(nadir_token_t *token,
                       const char *source,
                       const nadir_u64_t index) {
    token->string.value = source + index;
    token->string.count = 0;
}

bool nadir_token_increment(nadir_token_t *token) {
    nadir_u64_t maximum = NADIR_TOKEN_DEFAULT_MAXIMUM;
    if (token->kind == NADIR_TOKEN_KIND_NUMBER) {
        maximum = NADIR_TOKEN_NUMBER_MAXIMUM;
    } else if (token->kind == NADIR_TOKEN_KIND_PATH) {
        maximum = NADIR_TOKEN_PATH_MAXIMUM;
    }

    if (token->string.count < maximum) {
        ++token->string.count;
        return true;
    }

    return false;
}

const char *nadir_token_kind_encode(const nadir_token_kind_t kind) {
    switch (kind) {
        case NADIR_TOKEN_KIND_NUMBER:
            return "number";
        case NADIR_TOKEN_KIND_IDENT:
            return "identifier";
        case NADIR_TOKEN_KIND_PATH:
            return "path";
        case NADIR_TOKEN_KIND_COMPTIME:
            return "comptime";
        case NADIR_TOKEN_KIND_STORE_ADDRESS:
            return "store address";
        case NADIR_TOKEN_KIND_LOAD_ADDRESS:
            return "load address";
        case NADIR_TOKEN_KIND_UNTIL:
            return "until";
        case NADIR_TOKEN_KIND_REPEAT:
            return "repeat";
        case NADIR_TOKEN_KIND_LEFT_BRACE:
            return "{";
        case NADIR_TOKEN_KIND_RIGHT_BRACE:
            return "}";
        case NADIR_TOKEN_KIND_LEFT_PAREN:
            return "(";
        case NADIR_TOKEN_KIND_RIGHT_PAREN:
            return ")";
        case NADIR_TOKEN_KIND_EQUAL:
            return "=";
        case NADIR_TOKEN_KIND_COMMA:
            return ",";
        case NADIR_TOKEN_KIND_DOT:
            return ".";
        case NADIR_TOKEN_KIND_SEMICOLON:
            return ";";
        case NADIR_TOKEN_KIND_CONSTANT:
            return "constant";
        case NADIR_TOKEN_KIND_PROCEDURE:
            return "procedure";
        case NADIR_TOKEN_KIND_BINARY:
            return "binary";
        case NADIR_TOKEN_KIND_INCLUDE:
            return "include";
        case NADIR_TOKEN_KIND_TYPE_U8:
            return "u8";
        case NADIR_TOKEN_KIND_TYPE_U16:
            return "u16";
        case NADIR_TOKEN_KIND_TYPE_U32:
            return "u32";
        case NADIR_TOKEN_KIND_TYPE_U64:
            return "u64";
        case NADIR_TOKEN_KIND_TYPE_I8:
            return "i8";
        case NADIR_TOKEN_KIND_TYPE_I16:
            return "i16";
        case NADIR_TOKEN_KIND_TYPE_I32:
            return "i32";
        case NADIR_TOKEN_KIND_TYPE_I64:
            return "i64";
        case NADIR_TOKEN_KIND_EOF:
            return "EOF";
        default:
            return "unknown";
    }
}
