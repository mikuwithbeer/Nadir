#include "nadir/token.h"

int main(void) {
    nadir_token_list_t *token_list = nadir_token_list_new(1);

    auto token = nadir_token_new(NADIR_TOKEN_ID_IDENT, 1, 1);

    nadir_token_append(&token, 'a');
    nadir_token_append(&token, 'b');
    nadir_token_append(&token, 'c');

    nadir_token_list_append(token_list, token);

    token = nadir_token_new(NADIR_TOKEN_ID_NUMBER, 2, 2);

    nadir_token_append(&token, '1');
    nadir_token_append(&token, '2');
    nadir_token_append(&token, '3');
    token.specific.number = 123;

    nadir_token_list_append(token_list, token);

    token = nadir_token_new(NADIR_TOKEN_ID_EQUAL, 3, 3);

    nadir_token_append(&token, '=');

    nadir_token_list_append(token_list, token);

    token = nadir_token_new(NADIR_TOKEN_ID_EOF, 4, 4);

    nadir_token_list_append(token_list, token);

    nadir_token_list_free(token_list);
    return 0;
}
