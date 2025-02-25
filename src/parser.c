#include "parser.h"
#include <stdio.h>
#include <string.h>  // Untuk strdup()
#include <stdbool.h> // Untuk tipe bool

#if defined(__APPLE__) || defined(__linux__)
#include <strings.h>  // Untuk strdup() di beberapa sistem
#endif

// Forward declarations
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_function(Parser* parser);
static ASTNode* parse_primary(Parser* parser);
static ASTNode* parse_call(Parser* parser);
static bool expect_token(Parser* parser, TokenType type);

// Tambahkan di bagian awal file, setelah includes
static const char* built_in_functions[] = {
    "cetak",
    "http_get",
    "http_post",
    NULL
};

static bool is_built_in_function(const char* name) {
    for (int i = 0; built_in_functions[i] != NULL; i++) {
        if (strcmp(built_in_functions[i], name) == 0) {
            return true;
        }
    }
    return false;
}

static Token* advance_token(Parser* parser) {
    if (parser->current_token) {
        free_token(parser->current_token);
    }
    parser->current_token = get_next_token(parser->lexer);
    return parser->current_token;
}

static void parser_error(const char* message) {
    fprintf(stderr, "Parser Error: %s\n", message);
    exit(1);
}

static ASTNode* parse_expression(Parser* parser) {
    // Hapus atau comment out debug print
    /*printf("Parsing expression, current token type: %d, value: %s\n", 
           parser->current_token->type, 
           parser->current_token->value);*/

    switch (parser->current_token->type) {
        case TOKEN_NUMBER:
        case TOKEN_STRING:
        case TOKEN_IDENTIFIER:
            return parse_primary(parser);
        
        case TOKEN_ARRAY: {
            // Handle array declaration
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_LBRACKET) {
                parser_error("Expected '[' after 'array'");
                return NULL;
            }
            advance_token(parser);
            
            // Parse size expression
            ASTNode* size = parse_expression(parser);
            if (!size) return NULL;
            
            if (parser->current_token->type != TOKEN_RBRACKET) {
                parser_error("Expected ']' after array size");
                return NULL;
            }
            advance_token(parser);
            
            ASTNode* array_node = create_ast_node(AST_ARRAY_DECL, "array");
            add_child(array_node, size);
            return array_node;
        }
        
        case TOKEN_BENAR:
        case TOKEN_SALAH: {
            ASTNode* node = create_ast_node(AST_BOOLEAN, parser->current_token->value);
            advance_token(parser);
            return node;
        }
        
        default:
            parser_error("Expected expression");
            return NULL;
    }
}

static ASTNode* parse_statement(Parser* parser) {
    switch (parser->current_token->type) {
        case TOKEN_ISI: {
            advance_token(parser);
            if (parser->current_token->type != TOKEN_IDENTIFIER) {
                parser_error("Expected identifier after 'isi'");
                return NULL;
            }
            char* var_name = strdup(parser->current_token->value);
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_EQUALS) {
                free(var_name);
                parser_error("Expected '=' after variable name");
                return NULL;
            }
            advance_token(parser);
            
            ASTNode* expr = parse_expression(parser);
            if (!expr) {
                free(var_name);
                return NULL;
            }
            
            ASTNode* decl = create_ast_node(AST_VARIABLE_DECL, var_name);
            add_child(decl, expr);
            free(var_name);
            
            if (parser->current_token->type != TOKEN_SEMICOLON) {
                parser_error("Expected ';' after variable declaration");
                return NULL;
            }
            advance_token(parser);
            return decl;
        }
        
        case TOKEN_KEMBALI: {
            advance_token(parser);
            ASTNode* expr = parse_expression(parser);
            if (!expr) return NULL;
            
            ASTNode* ret = create_ast_node(AST_RETURN, NULL);
            add_child(ret, expr);
            
            if (parser->current_token->type != TOKEN_SEMICOLON) {
                parser_error("Expected ';' after return expression");
                return NULL;
            }
            advance_token(parser);
            return ret;
        }
        
        case TOKEN_CETAK: {
            advance_token(parser);
            if (parser->current_token->type != TOKEN_LPAREN) {
                parser_error("Expected '(' after 'cetak'");
                return NULL;
            }
            advance_token(parser);
            
            ASTNode* expr = parse_expression(parser);
            if (!expr) return NULL;
            
            if (parser->current_token->type != TOKEN_RPAREN) {
                parser_error("Expected ')' after expression");
                return NULL;
            }
            advance_token(parser);
            
            ASTNode* print = create_ast_node(AST_CALL, "cetak");
            add_child(print, expr);
            
            if (parser->current_token->type != TOKEN_SEMICOLON) {
                parser_error("Expected ';' after print statement");
                return NULL;
            }
            advance_token(parser);
            return print;
        }
        
        case TOKEN_KALO: {
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_LPAREN) {
                parser_error("Expected '(' after 'kalo'");
            }
            advance_token(parser);

            // Parse condition
            ASTNode* condition = create_ast_node(AST_BINARY_OP, ">");
            
            if (parser->current_token->type != TOKEN_IDENTIFIER) {
                parser_error("Expected identifier in condition");
            }
            ASTNode* left = create_ast_node(AST_IDENTIFIER, parser->current_token->value);
            advance_token(parser);

            if (parser->current_token->type != TOKEN_GREATER) {
                parser_error("Expected '>' in condition");
            }
            advance_token(parser);

            if (parser->current_token->type != TOKEN_NUMBER) {
                parser_error("Expected number in condition");
            }
            ASTNode* right = create_ast_node(AST_NUMBER, parser->current_token->value);
            advance_token(parser);

            add_child(condition, left);
            add_child(condition, right);

            if (parser->current_token->type != TOKEN_RPAREN) {
                parser_error("Expected ')'");
            }
            advance_token(parser);

            if (parser->current_token->type != TOKEN_LBRACE) {
                parser_error("Expected '{'");
            }
            advance_token(parser);

            // Parse if body
            ASTNode* if_node = create_ast_node(AST_IF, NULL);
            add_child(if_node, condition);

            ASTNode* body = create_ast_node(AST_BLOCK, NULL);
            while (parser->current_token->type != TOKEN_RBRACE) {
                ASTNode* stmt = parse_statement(parser);
                if (stmt) add_child(body, stmt);
            }
            add_child(if_node, body);
            
            advance_token(parser); // consume '}'
            return if_node;
        }
        
        case TOKEN_ULANG: {
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_LPAREN) {
                parser_error("Expected '(' after 'ulang'");
                return NULL;
            }
            advance_token(parser);
            
            ASTNode* condition = parse_expression(parser);
            if (!condition) return NULL;
            
            if (parser->current_token->type != TOKEN_RPAREN) {
                parser_error("Expected ')' after condition");
                return NULL;
            }
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_LBRACE) {
                parser_error("Expected '{' after condition");
                return NULL;
            }
            advance_token(parser);
            
            ASTNode* while_node = create_ast_node(AST_WHILE, NULL);
            add_child(while_node, condition);
            
            ASTNode* body = create_ast_node(AST_BLOCK, NULL);
            while (parser->current_token->type != TOKEN_RBRACE) {
                ASTNode* stmt = parse_statement(parser);
                if (stmt) add_child(body, stmt);
            }
            add_child(while_node, body);
            
            advance_token(parser);
            return while_node;
        }
        
        case TOKEN_IDENTIFIER: {
            char* var_name = strdup(parser->current_token->value);
            advance_token(parser);

            // Handle array access and assignment: arr[index] = value
            if (parser->current_token->type == TOKEN_LBRACKET) {
                advance_token(parser);
                ASTNode* index = parse_expression(parser);
                if (!index) {
                    free(var_name);
                    return NULL;
                }
                
                if (parser->current_token->type != TOKEN_RBRACKET) {
                    free(var_name);
                    parser_error("Expected ']' after array index");
                    return NULL;
                }
                advance_token(parser);
                
                if (parser->current_token->type == TOKEN_EQUALS) {
                    advance_token(parser);
                    ASTNode* assign = create_ast_node(AST_ARRAY_ASSIGN, var_name);
                    ASTNode* value = parse_expression(parser);
                    if (!value) {
                        free(var_name);
                        free_ast(index);
                        return NULL;
                    }
                    
                    add_child(assign, index);   // First child is index
                    add_child(assign, value);   // Second child is value
                    
                    if (parser->current_token->type != TOKEN_SEMICOLON) {
                        free(var_name);
                        parser_error("Expected ';' after array assignment");
                        return NULL;
                    }
                    advance_token(parser);
                    free(var_name);
                    return assign;
                }
            }

            if (parser->current_token->type == TOKEN_EQUALS) {
                advance_token(parser);
                ASTNode* assign = create_ast_node(AST_ASSIGNMENT, var_name);
                ASTNode* value = parse_expression(parser);
                add_child(assign, value);
                free(var_name);

                if (parser->current_token->type != TOKEN_SEMICOLON) {
                    parser_error("Expected ';'");
                }
                advance_token(parser);
                return assign;
            }
            free(var_name);
            break;
        }
        
        default:
            parser_error("Unexpected token in statement");
            return NULL;
    }
    return NULL;
}

static ASTNode* parse_function(Parser* parser) {
    DataType return_type = TYPE_VOID;  // Default return type
    
    // Parse function type
    switch (parser->current_token->type) {
        case TOKEN_BIKIN:
            advance_token(parser);
            if (parser->current_token->type != TOKEN_FUNGSI) {
                parser_error("Expected 'fungsi' after 'bikin'");
                return NULL;
            }
            advance_token(parser);
            break;
            
        case TOKEN_FUNGSI_INT:
            return_type = TYPE_INT;
            advance_token(parser);
            break;
            
        case TOKEN_FUNGSI_FLOAT:
            return_type = TYPE_FLOAT;
            advance_token(parser);
            break;
            
        case TOKEN_FUNGSI_BOOL:
            return_type = TYPE_BOOLEAN;
            advance_token(parser);
            break;
            
        case TOKEN_FUNGSI_STR:
            return_type = TYPE_STRING;
            advance_token(parser);
            break;
            
        default:
            parser_error("Expected 'bikin' or function type");
            return NULL;
    }

    // Parse function name
    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        parser_error("Expected function name");
        return NULL;
    }
    ASTNode* func = create_ast_node(AST_FUNCTION, parser->current_token->value);
    func->data_type = return_type;  // Set return type
    advance_token(parser);

    // Parse parameters
    if (parser->current_token->type != TOKEN_LPAREN) {
        parser_error("Expected '(' after function name");
        return NULL;
    }
    advance_token(parser);

    // Parse parameter list
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (parser->current_token->type != TOKEN_IDENTIFIER) {
            parser_error("Expected parameter name");
            return NULL;
        }
        
        // Get parameter name
        char* param_name = strdup(parser->current_token->value);
        advance_token(parser);
        
        // Expect colon
        if (parser->current_token->type != TOKEN_COLON) {
            free(param_name);
            parser_error("Expected ':' after parameter name");
            return NULL;
        }
        advance_token(parser);
        
        // Get parameter type
        DataType param_type;
        switch (parser->current_token->type) {
            case TOKEN_TYPE_INT:
                param_type = TYPE_INT;
                break;
            case TOKEN_TYPE_FLOAT:
                param_type = TYPE_FLOAT;
                break;
            case TOKEN_TYPE_BOOL:
                param_type = TYPE_BOOLEAN;
                break;
            case TOKEN_TYPE_STR:
                param_type = TYPE_STRING;
                break;
            default:
                free(param_name);
                parser_error("Expected parameter type");
                return NULL;
        }
        advance_token(parser);
        
        // Create parameter node
        ASTNode* param = create_ast_node(AST_PARAMETER, param_name);
        param->data_type = param_type;
        add_child(func, param);
        free(param_name);
        
        // Check for comma
        if (parser->current_token->type == TOKEN_COMMA) {
            advance_token(parser);
            continue;
        }
        
        // Break if we see closing parenthesis
        if (parser->current_token->type == TOKEN_RPAREN) {
            break;
        }
        
        parser_error("Expected ',' or ')' after parameter");
        return NULL;
    }
    advance_token(parser);

    // Parse function body
    if (parser->current_token->type != TOKEN_LBRACE) {
        parser_error("Expected '{' after parameters");
        return NULL;
    }
    advance_token(parser);

    // Parse function body statements
    ASTNode* body = create_ast_node(AST_BLOCK, NULL);
    while (parser->current_token->type != TOKEN_RBRACE) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            add_child(body, stmt);
        }
    }
    add_child(func, body);
    advance_token(parser);

    return func;
}

static ASTNode* parse_primary(Parser* parser) {
    Token* token = parser->current_token;
    ASTNode* node = NULL;

    switch (token->type) {
        case TOKEN_NUMBER: {
            node = create_ast_node(AST_NUMBER, token->value);
            advance_token(parser);
            return node;
        }
        
        case TOKEN_STRING: {
            node = create_ast_node(AST_STRING, token->value);
            advance_token(parser);
            return node;
        }
        
        case TOKEN_IDENTIFIER: {
            // Cek apakah ini function call
            if (is_built_in_function(token->value)) {
                return parse_call(parser);
            }
            
            node = create_ast_node(AST_IDENTIFIER, token->value);
            advance_token(parser);
            
            // Cek array access
            if (parser->current_token->type == TOKEN_LBRACKET) {
                ASTNode* array_access = create_ast_node(AST_ARRAY_ACCESS, node->value);
                advance_token(parser);
                
                ASTNode* index = parse_expression(parser);
                if (!index) return NULL;
                
                add_child(array_access, index);
                
                if (parser->current_token->type != TOKEN_RBRACKET) {
                    parser_error("Expected ']'");
                    return NULL;
                }
                advance_token(parser);
                
                free_ast(node);
                return array_access;
            }
            
            return node;
        }
        
        default:
            parser_error("Expected primary expression");
            return NULL;
    }
}

Parser* create_parser(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;

    parser->lexer = lexer;
    parser->current_token = NULL;
    advance_token(parser);
    return parser;
}

void free_parser(Parser* parser) {
    if (parser) {
        if (parser->current_token) {
            free_token(parser->current_token);
        }
        free(parser);
    }
}

ASTNode* parse(Parser* parser) {
    ASTNode* program = create_ast_node(AST_PROGRAM, NULL);
    
    while (parser->current_token && parser->current_token->type != TOKEN_EOF) {
        ASTNode* func = parse_function(parser);
        if (func) {
            add_child(program, func);
        }
    }
    
    return program;
}

static ASTNode* parse_call(Parser* parser) {
    const char* function_name = parser->current_token->value;
    
    if (!is_built_in_function(function_name)) {
        fprintf(stderr, "Error: Unknown function '%s'\n", function_name);
        return NULL;
    }
    
    ASTNode* node = create_ast_node(AST_CALL, function_name);
    
    // Consume function name
    advance_token(parser);
    
    // Expect opening parenthesis
    if (!expect_token(parser, TOKEN_LPAREN)) {
        free_ast(node);
        return NULL;
    }
    
    // Parse arguments
    while (parser->current_token->type != TOKEN_RPAREN) {
        ASTNode* arg = parse_expression(parser);
        if (!arg) {
            free_ast(node);
            return NULL;
        }
        add_child(node, arg);
        
        if (parser->current_token->type == TOKEN_COMMA) {
            advance_token(parser);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            fprintf(stderr, "Error: Expected ',' or ')'\n");
            free_ast(node);
            return NULL;
        }
    }
    
    // Consume closing parenthesis
    advance_token(parser);
    
    return node;
}

static bool expect_token(Parser* parser, TokenType type) {
    if (parser->current_token->type != type) {
        char message[100];
        snprintf(message, sizeof(message), "Expected token type %d, got %d", type, parser->current_token->type);
        parser_error(message);
        return false;
    }
    advance_token(parser);
    return true;
}
