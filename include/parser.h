#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token* current_token;
} Parser;

Parser* create_parser(Lexer* lexer);
void free_parser(Parser* parser);
ASTNode* parse(Parser* parser);

#endif
