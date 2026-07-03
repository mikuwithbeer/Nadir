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
            "    D = @div(Register.C, 7);\n"
            "}\n"
            "\n"
            "procedure jmp(u16, u8) {\n"
            "    10;\n"
            "    @bit_and(@at(0), 255);\n"
            "    @bit_and(@bit_shr(@at(0), 8), 255);\n"
            "}\n"
            "\n"
            "binary { <LOL; jmp(); }\n";
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
    const auto compiler_result = nadir_compiler_run(compiler);
    if (compiler_result.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        printf("Compile Error: %d, Token: %s, Line: %llu, Column: %llu\n", compiler_result.kind,
               compiler_result.token->value,
               compiler_result.token->line, compiler_result.token->column);
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

    nadir_compiler_free(compiler);
    nadir_parser_free(parser);
    nadir_lexer_free(lexer);
    return 0;
}
