#include <stdio.h>
#include <string.h>

#include "nadir/lexer.h"

int main(void) {
    const char *source =
            "const Reg1ster {}() ;/\n\n5__12_4\n{; lo123l ()\n } \nmap instruction\n ()()()(}213 = -10\n +4 ";
    const auto lexer = nadir_lexer_new(source, strlen(source));
    const auto result = nadir_lexer_collect(lexer);

    if (result.id != NADIR_LEXER_ERROR_ID_NONE) {
        printf("Error: %d, Line: %llu, Column: %llu\n", result.id, result.line, result.column);
        nadir_lexer_free(lexer);
        return 1;
    }

    for (nadir_u64_t i = 0; i < lexer->token_list->token_count; ++i) {
        const auto token = lexer->token_list->tokens[i];
        printf("%llu:%llu %d %s\n", token.line, token.column, token.id, token.value);
    }

    nadir_lexer_free(lexer);
    return 0;
}
