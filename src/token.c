#include <stdlib.h>
#include <string.h>
#include "token.h"

Token* create_token(TokenType type, const char* value, int line, int column) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) {
        return NULL;
    }

    token->type = type;
    token->line = line;
    token->column = column;

    if (value) {
        // Alokasi memori untuk value dan copy string
        token->value = (char*)malloc(strlen(value) + 1);
        if (!token->value) {
            free(token);
            return NULL;
        }
        strcpy(token->value, value);
    } else {
        token->value = NULL;
    }

    return token;
}

void free_token(Token* token) {
    if (token) {
        // Bebaskan memori untuk value jika ada
        if (token->value) {
            free(token->value);
        }
        // Bebaskan memori untuk struct token
        free(token);
    }
}
