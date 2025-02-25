#ifndef AST_H
#define AST_H

#include "types.h"
#include <stdlib.h>

// Tipe node AST
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
    AST_CALL,
    AST_STRING,
    AST_NUMBER,
    AST_FLOAT,           // Tipe data baru
    AST_BOOLEAN,         // Tipe data baru
    AST_IDENTIFIER,
    AST_BINARY_OP,
    AST_VARIABLE_DECL,
    AST_IF,
    AST_WHILE,        // untuk ulang
    AST_ASSIGNMENT,    // untuk i = i + 1
    AST_PARAMETER,       // Untuk parameter fungsi
    AST_RETURN,         // Untuk return statement
    AST_FUNCTION_DECL,   // Untuk deklarasi fungsi
    AST_ARRAY_DECL,    // Deklarasi array
    AST_ARRAY_ACCESS,  // Akses elemen array
    AST_ARRAY_ASSIGN,  // Assignment ke array
    AST_ARRAY_LITERAL  // Nilai array langsung
} ASTNodeType;

// Struktur untuk parameter fungsi
typedef struct {
    char* name;
    DataType type;
} Parameter;

// Struktur dasar untuk node AST
typedef struct ASTNode {
    ASTNodeType type;
    char* value;
    DataType data_type;  // Tambahkan tipe data
    struct ASTNode** children;
    int children_count;
    Parameter* parameters;  // Untuk menyimpan parameter fungsi
    int param_count;
} ASTNode;

// Fungsi-fungsi untuk membuat node AST
ASTNode* create_ast_node(ASTNodeType type, const char* value);
void add_child(ASTNode* parent, ASTNode* child);
void free_ast(ASTNode* node);

#endif 