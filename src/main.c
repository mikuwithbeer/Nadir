#include <stdio.h>
#include <string.h>

#include "nadir/lexer.h"

int main(void) {
    const char *source =
            "const Register {\n"
            "    A = 0;\n"
            "    B = 1;\n"
            "#   C = 2;\n"
            "}\n"
            "\n"
            "# Working? 123... Test!\n"
            "procedure jmp(u16) {\n"
            "    10;\n"
            "    @bitAnd(@at(0), 255);\n"
            "    @bitAnd(@bitShr(@at(0), 8), 255);\n"
            "}\n"
            "\n"
            "procedure main() {\n"
            "    3;\n"
            "    add(Register.A, 11, 30);\n"
            "    add(Register.# boo hoo\n"
            "    B, +10, -100);"
            "}\n";

    const auto lexer = nadir_lexer_new(source, strlen(source));
    const auto result = nadir_lexer_collect(lexer);

    if (result.id != NADIR_LEXER_ERROR_ID_NONE) {
        printf("Error: %d, Line: %llu, Column: %llu\n", result.id, result.line, result.column);
        nadir_lexer_free(lexer);
        return 1;
    }

    for (nadir_u64_t i = 0; i < lexer->token_list->token_count; ++i) {
        const auto token = lexer->token_list->tokens[i];
        printf("%llu:%llu ", token.line, token.column);

        switch (token.id) {
            case NADIR_TOKEN_ID_EOF:
                printf("EOF");
                break;
            case NADIR_TOKEN_ID_NUMBER:
                printf("NUMBER(%s)", token.value);
                break;
            case NADIR_TOKEN_ID_IDENT:
                printf("IDENT(%s)", token.value);
                break;
            case NADIR_TOKEN_ID_BUILTIN:
                printf("BUILTIN(%s)", token.value);
                break;
            case NADIR_TOKEN_ID_CONST:
                printf("CONST");
                break;
            case NADIR_TOKEN_ID_PROCEDURE:
                printf("PROCEDURE");
                break;
            case NADIR_TOKEN_ID_LEFT_BRACE:
                printf("{");
                break;
            case NADIR_TOKEN_ID_RIGHT_BRACE:
                printf("}");
                break;
            case NADIR_TOKEN_ID_LEFT_PAREN:
                printf("(");
                break;
            case NADIR_TOKEN_ID_RIGHT_PAREN:
                printf(")");
                break;
            case NADIR_TOKEN_ID_EQUAL:
                printf("=");
                break;
            case NADIR_TOKEN_ID_COMMA:
                printf(",");
                break;
            case NADIR_TOKEN_ID_DOT:
                printf(".");
                break;
            case NADIR_TOKEN_ID_SEMICOLON:
                printf(";");
                break;
            case NADIR_TOKEN_ID_TYPE_U8:
                printf("TYPE_U8");
                break;
            case NADIR_TOKEN_ID_TYPE_U16:
                printf("TYPE_U16");
                break;
            case NADIR_TOKEN_ID_TYPE_U32:
                printf("TYPE_U32");
                break;
            case NADIR_TOKEN_ID_TYPE_U64:
                printf("TYPE_U64");
                break;
            case NADIR_TOKEN_ID_TYPE_I8:
                printf("TYPE_I8");
                break;
            case NADIR_TOKEN_ID_TYPE_I16:
                printf("TYPE_I16");
                break;
            case NADIR_TOKEN_ID_TYPE_I32:
                printf("TYPE_I32");
                break;
            case NADIR_TOKEN_ID_TYPE_I64:
                printf("TYPE_I64");
                break;
        }

        printf("\n");
    }

    nadir_lexer_free(lexer);
    return 0;
}
