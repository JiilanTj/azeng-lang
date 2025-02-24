#include "parser.h"
#include <stdio.h>
#include <string.h>  // Untuk strdup()

#if defined(__APPLE__) || defined(__linux__)
#include <strings.h>  // Untuk strdup() di beberapa sistem
#endif

// Forward declarations
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_function(Parser* parser);

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
    // Parse left side
    ASTNode* left;
    if (parser->current_token->type == TOKEN_NUMBER) {
        left = create_ast_node(AST_NUMBER, parser->current_token->value);
        advance_token(parser);
    } else if (parser->current_token->type == TOKEN_IDENTIFIER) {
        left = create_ast_node(AST_IDENTIFIER, parser->current_token->value);
        advance_token(parser);
    } else {
        parser_error("Expected number or identifier in expression");
        return NULL;
    }

    // If there's no operator, return just the left side
    if (parser->current_token->type != TOKEN_PLUS && 
        parser->current_token->type != TOKEN_MINUS &&
        parser->current_token->type != TOKEN_MULTIPLY &&
        parser->current_token->type != TOKEN_DIVIDE &&
        parser->current_token->type != TOKEN_LESS &&
        parser->current_token->type != TOKEN_GREATER) {
        return left;
    }

    // Parse operator
    char* op;
    switch (parser->current_token->type) {
        case TOKEN_PLUS: op = "+"; break;
        case TOKEN_MINUS: op = "-"; break;
        case TOKEN_MULTIPLY: op = "*"; break;
        case TOKEN_DIVIDE: op = "/"; break;
        case TOKEN_LESS: op = "<"; break;
        case TOKEN_GREATER: op = ">"; break;
        default: 
            parser_error("Unknown operator");
            return NULL;
    }
    advance_token(parser);

    // Parse right side
    ASTNode* right;
    if (parser->current_token->type == TOKEN_NUMBER) {
        right = create_ast_node(AST_NUMBER, parser->current_token->value);
        advance_token(parser);
    } else if (parser->current_token->type == TOKEN_IDENTIFIER) {
        right = create_ast_node(AST_IDENTIFIER, parser->current_token->value);
        advance_token(parser);
    } else {
        parser_error("Expected number or identifier after operator");
        return NULL;
    }

    // Create and return binary operation node
    ASTNode* binary = create_ast_node(AST_BINARY_OP, op);
    add_child(binary, left);
    add_child(binary, right);
    return binary;
}

static ASTNode* parse_statement(Parser* parser) {
    switch (parser->current_token->type) {
        case TOKEN_CETAK: {
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_LPAREN) {
                parser_error("Expected '(' after 'cetak'");
            }
            advance_token(parser);

            ASTNode* print_call = create_ast_node(AST_CALL, "cetak");
            
            // Parse argument (string atau identifier)
            if (parser->current_token->type == TOKEN_STRING || 
                parser->current_token->type == TOKEN_IDENTIFIER) {
                ASTNode* arg = create_ast_node(
                    parser->current_token->type == TOKEN_STRING ? AST_STRING : AST_IDENTIFIER,
                    parser->current_token->value
                );
                add_child(print_call, arg);
            } else {
                parser_error("Expected string or identifier argument");
            }
            advance_token(parser);

            if (parser->current_token->type != TOKEN_RPAREN) {
                parser_error("Expected ')'");
            }
            advance_token(parser);

            if (parser->current_token->type != TOKEN_SEMICOLON) {
                parser_error("Expected ';'");
            }
            advance_token(parser);

            return print_call;
        }
        
        case TOKEN_ISI: {
            advance_token(parser);
            
            if (parser->current_token->type != TOKEN_IDENTIFIER) {
                parser_error("Expected identifier after 'isi'");
            }
            char* var_name = strdup(parser->current_token->value);
            advance_token(parser);

            if (parser->current_token->type != TOKEN_EQUALS) {
                free(var_name);
                parser_error("Expected '=' in variable declaration");
            }
            advance_token(parser);

            ASTNode* var_decl = create_ast_node(AST_VARIABLE_DECL, var_name);
            free(var_name);

            // Parse the value expression
            ASTNode* value = parse_expression(parser);
            if (!value) {
                parser_error("Invalid expression in variable declaration");
            }
            add_child(var_decl, value);

            if (parser->current_token->type != TOKEN_SEMICOLON) {
                parser_error("Expected ';'");
            }
            advance_token(parser);

            return var_decl;
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
            }
            advance_token(parser);

            ASTNode* condition = parse_expression(parser);
            if (!condition) {
                parser_error("Invalid loop condition");
            }

            if (parser->current_token->type != TOKEN_RPAREN) {
                parser_error("Expected ')'");
            }
            advance_token(parser);

            if (parser->current_token->type != TOKEN_LBRACE) {
                parser_error("Expected '{'");
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
            
            advance_token(parser); // consume '}'
            return while_node;
        }
        
        case TOKEN_IDENTIFIER: {
            char* var_name = strdup(parser->current_token->value);
            advance_token(parser);

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

static DataType get_type_from_token(TokenType token_type) {
    switch (token_type) {
        case TOKEN_TYPE_INT: return TYPE_INT;
        case TOKEN_TYPE_FLOAT: return TYPE_FLOAT;
        case TOKEN_TYPE_BOOL: return TYPE_BOOLEAN;
        case TOKEN_TYPE_STR: return TYPE_STRING;
        default: return TYPE_VOID;
    }
}

static ASTNode* parse_function(Parser* parser) {
    // Debug: print current token
    printf("Current token type: %d, value: %s\n", 
           parser->current_token->type, 
           parser->current_token->value ? parser->current_token->value : "NULL");
    
    // Parse function type
    DataType return_type = TYPE_VOID;  // default untuk fungsi biasa
    
    // Handle function type or bikin fungsi
    if (parser->current_token->type == TOKEN_BIKIN) {
        advance_token(parser);
        if (parser->current_token->type != TOKEN_FUNGSI) {
            parser_error("Expected 'fungsi' after 'bikin'");
        }
        advance_token(parser);
    } else {
        switch (parser->current_token->type) {
            case TOKEN_FUNGSI:
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
                parser_error("Expected function type or 'bikin fungsi'");
        }
    }

    // Debug: print token after function type
    printf("Token after function type: %d, value: %s\n", 
           parser->current_token->type, 
           parser->current_token->value ? parser->current_token->value : "NULL");

    // Parse function name
    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        parser_error("Expected function name");
    }
    
    ASTNode* func = create_ast_node(AST_FUNCTION, parser->current_token->value);
    func->data_type = return_type;
    advance_token(parser);
    
    // Parse parameters
    if (parser->current_token->type != TOKEN_LPAREN) {
        parser_error("Expected '(' after function name");
    }
    advance_token(parser);
    
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (parser->current_token->type != TOKEN_IDENTIFIER) {
            parser_error("Expected parameter name");
        }
        
        char* param_name = strdup(parser->current_token->value);
        advance_token(parser);
        
        // Parse parameter type
        if (parser->current_token->type != TOKEN_COLON) {
            free(param_name);
            parser_error("Expected ':' after parameter name");
        }
        advance_token(parser);
        
        // Get parameter type
        DataType param_type = get_type_from_token(parser->current_token->type);
        if (param_type == TYPE_VOID) {
            free(param_name);
            parser_error("Invalid parameter type");
        }
        advance_token(parser);
        
        // Create parameter node
        ASTNode* param = create_ast_node(AST_PARAMETER, param_name);
        param->data_type = param_type;
        add_child(func, param);
        free(param_name);
        
        if (parser->current_token->type == TOKEN_COMMA) {
            advance_token(parser);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            parser_error("Expected ',' or ')'");
        }
    }
    advance_token(parser);  // consume ')'
    
    // Parse function body
    if (parser->current_token->type != TOKEN_LBRACE) {
        parser_error("Expected '{' after function parameters");
    }
    advance_token(parser);
    
    ASTNode* body = create_ast_node(AST_BLOCK, NULL);
    while (parser->current_token->type != TOKEN_RBRACE) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) add_child(body, stmt);
    }
    add_child(func, body);
    
    advance_token(parser);  // consume '}'
    return func;
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
