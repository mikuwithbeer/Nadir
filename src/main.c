#include <stdio.h>
#include <string.h>

#include "nadir/lexer.h"

int main(void) {
    const char *source = "AbcLol\n5124\n{; lol ()\n } wow 41";
    const auto lexer = nadir_lexer_new(source, strlen(source));
    const auto result = nadir_lexer_run(lexer);

    if (result.id != NADIR_LEXER_ERROR_ID_NONE) {
        printf("Lexer error at line %llu, column %llu: unexpected character '%c'\n",
               result.line, result.column, result.specific.unexpected_character);

        nadir_lexer_free(lexer);
        return 1;
    }

    for (nadir_u64_t i = 0; i < lexer->token_list->token_count; ++i) {
        const auto token = lexer->token_list->tokens[i];
        printf("%d %s\n", token.id, token.value);
    }

    nadir_lexer_free(lexer);
    return 0;
}
