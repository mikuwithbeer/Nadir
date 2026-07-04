#include <stdio.h>
#include <string.h>

#include "nadir/compiler.h"
#include "nadir/lexer.h"
#include "nadir/parser.h"

int main(void) {
    const char *source =
            "constant Register {\n"
            "    A = 1;\n"
            "    B = @mul(Register.A, 10);\n"
            "    C = @mul(Register.B, 10);\n"
            "}\n"
            "procedure jmp(u8, u8) {\n"
            "    10;\n"
            "    @bit_or(4, @bit_or(2, 1));\n"
            "    @at(1);\n"
            "    @bit_shl(@at(0), 1);\n"
            "}\n"
            "procedure trying(u64, u8, u8) {\n"
            "    31; @at(0);\n"
            "    @mul(@at(1), 2);\n"
            "    @at(2);\n"
            "    @cast(-100, u8);\n"
            "}\n"
            "binary { jmp(16, @mod(@add(Register.A, 1337), Other.Second)); <WOW; trying(>WOW, 5, 10); }\n"
            "constant Other { First = @mod(10, 3); Second = @add(Other.First, u32); }";

    const auto lexer = nadir_lexer_new(source, strlen(source));
    const auto lexer_result = nadir_lexer_collect(lexer);
    if (lexer_result.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        printf("Token Error: %d, Line: %llu, Column: %llu\n", lexer_result.kind, lexer_result.line,
               lexer_result.column);
        nadir_lexer_free(lexer);
        return 1;
    }

    const auto parser = nadir_parser_new(lexer->tokens);
    const auto parser_result = nadir_parser_run(parser);
    if (parser_result.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        printf("Parse Error: %d, Token: %s, Line: %llu, Column: %llu\n", parser_result.kind, parser_result.token->value,
               parser_result.token->line, parser_result.token->column);
        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        return 1;
    }

    const auto compiler = nadir_compiler_new(parser->ast);
    auto compiler_result = nadir_compiler_prepare(compiler);
    if (compiler_result.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        printf("Prepare Error: %d, Token: %s, Line: %llu, Column: %llu\n", compiler_result.kind,
               compiler_result.token->value,
               compiler_result.token->line, compiler_result.token->column);
        nadir_compiler_free(compiler);
        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        return 1;
    }

    compiler_result = nadir_compiler_run(compiler);
    if (compiler_result.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        printf("Compile Error: %d, Token: %s, Line: %llu, Column: %llu\n", compiler_result.kind,
               compiler_result.token->value,
               compiler_result.token->line, compiler_result.token->column);
        nadir_compiler_free(compiler);
        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        return 1;
    }

    for (nadir_u64_t index = 0; index < compiler->constants->capacity; ++index) {
        if (compiler->constants->entries[index].is_used) {
            const auto key = compiler->constants->entries[index].key;
            const auto value = (nadir_compiler_constant_t *) compiler->constants->entries[index].value;

            const auto high = (nadir_u64_t) (value->value >> 64);
            const auto low = (nadir_u64_t) value->value;

            printf("%s = 0x%016llx%016llx\n", key, high, low);
        }
    }

    for (nadir_u64_t index = 0; index < compiler->procedures->capacity; ++index) {
        if (compiler->procedures->entries[index].is_used) {
            const auto key = compiler->procedures->entries[index].key;
            const auto value = (nadir_compiler_procedure_t *) compiler->procedures->entries[index].value;

            printf("%s/%llu: %llu statements\n", key, value->parameters->length, value->statements->length);
        }
    }

    for (nadir_u64_t index = 0; index < compiler->addresses->capacity; ++index) {
        if (compiler->addresses->entries[index].is_used) {
            const auto key = compiler->addresses->entries[index].key;
            const auto value = (nadir_u64_t *) compiler->addresses->entries[index].value;

            printf("<%s = %llu\n", key, *value);
        }
    }

    for (nadir_u64_t index = 0; index < compiler->output->length; ++index) {
        const auto byte = *(nadir_u8_t *) nadir_list_get(compiler->output, index);
        printf("%02x ", byte);
    }

    printf("\n");

    nadir_compiler_free(compiler);
    nadir_parser_free(parser);
    nadir_lexer_free(lexer);
    return 0;
}
