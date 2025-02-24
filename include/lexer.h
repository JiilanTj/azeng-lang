#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
    char* source;
    int position;
    int line;
    int column;
} Lexer;

Lexer* create_lexer(const char* source);
Token* get_next_token(Lexer* lexer);
void free_lexer(Lexer* lexer);

#endif
