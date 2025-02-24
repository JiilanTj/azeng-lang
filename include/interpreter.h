#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

#define MAX_VARIABLES 100

typedef struct {
    char* name;
    char* value;
    DataType type;
} Variable;

typedef struct Interpreter {
    Variable variables[MAX_VARIABLES];
    int variable_count;
} Interpreter;

typedef struct {
    DataType type;
    char* value;
} Value;

Interpreter* create_interpreter(void);
void free_interpreter(Interpreter* interpreter);
void interpret(Interpreter* interpreter, ASTNode* node);
Value* create_value(DataType type, const char* value);
void free_value(Value* value);

#endif