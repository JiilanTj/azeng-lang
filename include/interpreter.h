#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

#define MAX_VARIABLES 100

typedef struct {
    char* name;
    int value;
} Variable;

typedef struct Interpreter {
    Variable variables[MAX_VARIABLES];
    int variable_count;
} Interpreter;

Interpreter* create_interpreter(void);
void free_interpreter(Interpreter* interpreter);
void interpret(Interpreter* interpreter, ASTNode* node);

#endif