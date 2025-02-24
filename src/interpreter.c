#include "interpreter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declarations
static Value* evaluate_expression(Interpreter* interpreter, ASTNode* node);
static void interpret_block(Interpreter* interpreter, ASTNode* node);
static void interpret_function(Interpreter* interpreter, ASTNode* node);

// Implementasi fungsi untuk Value
Value* create_value(DataType type, const char* value) {
    Value* val = (Value*)malloc(sizeof(Value));
    if (!val) return NULL;
    
    val->type = type;
    val->value = strdup(value);
    return val;
}

void free_value(Value* value) {
    if (value) {
        free(value->value);
        free(value);
    }
}

Interpreter* create_interpreter(void) {
    Interpreter* interpreter = (Interpreter*)malloc(sizeof(Interpreter));
    if (!interpreter) return NULL;
    interpreter->variable_count = 0;
    return interpreter;
}

void free_interpreter(Interpreter* interpreter) {
    if (interpreter) {
        for (int i = 0; i < interpreter->variable_count; i++) {
            free(interpreter->variables[i].name);
        }
        free(interpreter);
    }
}

// Modifikasi get_variable untuk mengembalikan Value*
static Value* get_variable(Interpreter* interpreter, const char* name) {
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            return create_value(interpreter->variables[i].type, 
                              interpreter->variables[i].value);
        }
    }
    fprintf(stderr, "Error: Variable '%s' not found\n", name);
    exit(1);
}

// Modifikasi set_variable untuk menerima Value*
static void set_variable(Interpreter* interpreter, const char* name, Value* value) {
    // Check if variable exists
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            free(interpreter->variables[i].value);
            interpreter->variables[i].value = strdup(value->value);
            interpreter->variables[i].type = value->type;
            return;
        }
    }
    
    // Create new variable
    if (interpreter->variable_count < MAX_VARIABLES) {
        interpreter->variables[interpreter->variable_count].name = strdup(name);
        interpreter->variables[interpreter->variable_count].value = strdup(value->value);
        interpreter->variables[interpreter->variable_count].type = value->type;
        interpreter->variable_count++;
    } else {
        fprintf(stderr, "Error: Too many variables\n");
        exit(1);
    }
}

static Value* evaluate_expression(Interpreter* interpreter, ASTNode* node) {
    if (!node) return create_value(TYPE_VOID, "0");

    switch (node->type) {
        case AST_NUMBER:
            return create_value(TYPE_INT, node->value);
            
        case AST_FLOAT:
            return create_value(TYPE_FLOAT, node->value);
            
        case AST_STRING:
            return create_value(TYPE_STRING, node->value);
            
        case AST_BOOLEAN:
            return create_value(TYPE_BOOLEAN, node->value);
            
        case AST_IDENTIFIER:
            return get_variable(interpreter, node->value);
            
        case AST_CALL: {
            Value *left = NULL, *right = NULL, *result = NULL;
            
            if (node->children_count == 2) {
                left = evaluate_expression(interpreter, node->children[0]);
                right = evaluate_expression(interpreter, node->children[1]);
                
                if (strcmp(node->value, "tambah") == 0) {
                    if (left->type == TYPE_INT && right->type == TYPE_INT) {
                        int val = atoi(left->value) + atoi(right->value);
                        char str_result[32];
                        sprintf(str_result, "%d", val);
                        result = create_value(TYPE_INT, str_result);
                    }
                }
                else if (strcmp(node->value, "bagi") == 0) {
                    if (left->type == TYPE_FLOAT && right->type == TYPE_FLOAT) {
                        float val = atof(left->value) / atof(right->value);
                        char str_result[32];
                        sprintf(str_result, "%.2f", val);
                        result = create_value(TYPE_FLOAT, str_result);
                    }
                }
                else if (strcmp(node->value, "lebih_besar") == 0) {
                    if (left->type == TYPE_INT && right->type == TYPE_INT) {
                        int val = atoi(left->value) > atoi(right->value);
                        result = create_value(TYPE_BOOLEAN, val ? "benar" : "salah");
                    }
                }
                else if (strcmp(node->value, "gabung") == 0) {
                    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
                        char* str_result = malloc(strlen(left->value) + strlen(right->value) + 1);
                        strcpy(str_result, left->value);
                        strcat(str_result, right->value);
                        result = create_value(TYPE_STRING, str_result);
                        free(str_result);
                    }
                }
            }
            
            if (left) free_value(left);
            if (right) free_value(right);
            return result ? result : create_value(TYPE_VOID, "0");
        }
            
        case AST_BINARY_OP: {
            Value *left = evaluate_expression(interpreter, node->children[0]);
            Value *right = evaluate_expression(interpreter, node->children[1]);
            Value *result = NULL;
            
            if (left->type == TYPE_INT && right->type == TYPE_INT) {
                int l = atoi(left->value);
                int r = atoi(right->value);
                int val;
                char str_result[32];
                
                if (strcmp(node->value, "+") == 0) val = l + r;
                else if (strcmp(node->value, "-") == 0) val = l - r;
                else if (strcmp(node->value, "*") == 0) val = l * r;
                else if (strcmp(node->value, "/") == 0) val = l / r;
                else if (strcmp(node->value, "<") == 0) {
                    result = create_value(TYPE_BOOLEAN, l < r ? "benar" : "salah");
                }
                else if (strcmp(node->value, ">") == 0) {
                    result = create_value(TYPE_BOOLEAN, l > r ? "benar" : "salah");
                }
                
                if (!result) {
                    sprintf(str_result, "%d", val);
                    result = create_value(TYPE_INT, str_result);
                }
            }
            else if (left->type == TYPE_FLOAT && right->type == TYPE_FLOAT) {
                float l = atof(left->value);
                float r = atof(right->value);
                float val;
                char str_result[32];
                
                if (strcmp(node->value, "+") == 0) val = l + r;
                else if (strcmp(node->value, "-") == 0) val = l - r;
                else if (strcmp(node->value, "*") == 0) val = l * r;
                else if (strcmp(node->value, "/") == 0) val = l / r;
                
                sprintf(str_result, "%.2f", val);
                result = create_value(TYPE_FLOAT, str_result);
            }
            else if (left->type == TYPE_STRING && right->type == TYPE_STRING &&
                     strcmp(node->value, "+") == 0) {
                char* str_result = malloc(strlen(left->value) + strlen(right->value) + 1);
                strcpy(str_result, left->value);
                strcat(str_result, right->value);
                result = create_value(TYPE_STRING, str_result);
                free(str_result);
            }
            
            free_value(left);
            free_value(right);
            return result ? result : create_value(TYPE_VOID, "0");
        }
        
        default:
            return create_value(TYPE_VOID, "0");
    }
}

static void interpret_call(Interpreter* interpreter, ASTNode* node) {
    if (strcmp(node->value, "cetak") == 0 && node->children_count > 0) {
        Value* val = evaluate_expression(interpreter, node->children[0]);
        printf("%s\n", val->value);
        free_value(val);
    }
}

// Modifikasi interpret_variable_decl dan interpret_assignment
static void interpret_variable_decl(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        Value* value = evaluate_expression(interpreter, node->children[0]);
        set_variable(interpreter, node->value, value);
        free_value(value);
    }
}

static void interpret_assignment(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        Value* value = evaluate_expression(interpreter, node->children[0]);
        set_variable(interpreter, node->value, value);
        free_value(value);
    }
}

static void interpret_block(Interpreter* interpreter, ASTNode* node) {
    for (int i = 0; i < node->children_count; i++) {
        interpret(interpreter, node->children[i]);
    }
}

static void interpret_function(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        interpret(interpreter, node->children[0]);
    }
}

static void interpret_if(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count >= 2) {
        ASTNode* condition = node->children[0];
        ASTNode* body = node->children[1];
        
        if (evaluate_expression(interpreter, condition)) {
            interpret(interpreter, body);
        }
    }
}

static void interpret_while(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count >= 2) {  // Harus punya kondisi dan body
        while (1) {
            Value* condition = evaluate_expression(interpreter, node->children[0]);
            int continue_loop = strcmp(condition->value, "benar") == 0;
            free_value(condition);
            
            if (!continue_loop) break;
            
            interpret_block(interpreter, node->children[1]);
        }
    }
}

void interpret(Interpreter* interpreter, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->children_count; i++) {
                interpret(interpreter, node->children[i]);
            }
            break;
            
        case AST_FUNCTION:
            interpret_function(interpreter, node);
            break;
            
        case AST_BLOCK:
            interpret_block(interpreter, node);
            break;
            
        case AST_CALL:
            interpret_call(interpreter, node);
            break;
            
        case AST_VARIABLE_DECL:
            interpret_variable_decl(interpreter, node);
            break;
            
        case AST_IF:
            interpret_if(interpreter, node);
            break;
            
        case AST_WHILE:
            interpret_while(interpreter, node);
            break;
            
        case AST_ASSIGNMENT:
            interpret_assignment(interpreter, node);
            break;
            
        default:
            break;
    }
} 