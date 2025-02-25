#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdbool.h>
#include "types.h"
#include "ast.h"

#define MAX_VARIABLES 100

typedef struct {
    DataType type;
    union {
        int int_val;
        float float_val;
        bool bool_val;
        char* str_val;
        int* int_array;     
        float* float_array; 
        char** str_array;   
        bool* bool_array;    
    } value;
    int array_size;        
} Value;

typedef struct {
    char* name;
    Value* value;
} Variable;

typedef struct Interpreter {
    Variable variables[MAX_VARIABLES];
    int variable_count;
} Interpreter;

// Function declarations
Interpreter* create_interpreter(void);
void free_interpreter(Interpreter* interpreter);
void interpret(Interpreter* interpreter, ASTNode* node);
Value* create_array(DataType type, int size);
void free_value(Value* value);

// HTTP functions
Value evaluate_http_get(const char* url);
Value evaluate_http_post(const char* url, const char* data);

#endif