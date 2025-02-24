#include "ast.h"
#include <string.h>

ASTNode* create_ast_node(ASTNodeType type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;

    node->type = type;
    node->children = NULL;
    node->children_count = 0;

    if (value) {
        node->value = strdup(value);
    } else {
        node->value = NULL;
    }

    return node;
}

void add_child(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;

    parent->children_count++;
    parent->children = (ASTNode**)realloc(
        parent->children, 
        parent->children_count * sizeof(ASTNode*)
    );
    
    if (parent->children) {
        parent->children[parent->children_count - 1] = child;
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;

    for (int i = 0; i < node->children_count; i++) {
        free_ast(node->children[i]);
    }

    if (node->children) free(node->children);
    if (node->value) free(node->value);
    free(node);
} 