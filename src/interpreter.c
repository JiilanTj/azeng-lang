#include "interpreter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declarations
static int evaluate_expression(Interpreter* interpreter, ASTNode* node);
static void interpret_block(Interpreter* interpreter, ASTNode* node);
static void interpret_function(Interpreter* interpreter, ASTNode* node);

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

static void set_variable(Interpreter* interpreter, const char* name, int value) {
    // Check if variable exists
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            interpreter->variables[i].value = value;
            return;
        }
    }
    
    // Create new variable
    if (interpreter->variable_count < MAX_VARIABLES) {
        interpreter->variables[interpreter->variable_count].name = strdup(name);
        interpreter->variables[interpreter->variable_count].value = value;
        interpreter->variable_count++;
    }
}

static int get_variable(Interpreter* interpreter, const char* name) {
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            return interpreter->variables[i].value;
        }
    }
    fprintf(stderr, "Error: Variable '%s' not found\n", name);
    exit(1);
}

static int evaluate_expression(Interpreter* interpreter, ASTNode* node) {
    if (!node) return 0;

    switch (node->type) {
        case AST_NUMBER:
            return atoi(node->value);
            
        case AST_IDENTIFIER:
            return get_variable(interpreter, node->value);
            
        case AST_BINARY_OP:
            if (node->children_count != 2) return 0;
            
            int left = evaluate_expression(interpreter, node->children[0]);
            int right = evaluate_expression(interpreter, node->children[1]);
            
            if (strcmp(node->value, "+") == 0) return left + right;
            if (strcmp(node->value, "-") == 0) return left - right;
            if (strcmp(node->value, "*") == 0) return left * right;
            if (strcmp(node->value, "/") == 0) return left / right;
            if (strcmp(node->value, "<") == 0) return left < right;
            if (strcmp(node->value, ">") == 0) return left > right;
            break;
            
        default:
            return 0;
    }
    return 0;
}

static void interpret_call(Interpreter* interpreter, ASTNode* node) {
    if (strcmp(node->value, "cetak") == 0 && node->children_count > 0) {
        ASTNode* arg = node->children[0];
        if (arg->type == AST_STRING) {
            printf("%s\n", arg->value);
        } else if (arg->type == AST_IDENTIFIER) {
            printf("%d\n", get_variable(interpreter, arg->value));
        }
    }
}

static void interpret_variable_decl(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        int value = evaluate_expression(interpreter, node->children[0]);
        set_variable(interpreter, node->value, value);
    }
}

static void interpret_assignment(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        int value = evaluate_expression(interpreter, node->children[0]);
        set_variable(interpreter, node->value, value);
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
    if (node->children_count >= 2) {
        ASTNode* condition = node->children[0];
        ASTNode* body = node->children[1];
        
        while (evaluate_expression(interpreter, condition)) {
            interpret(interpreter, body);
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