#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

Lexer* create_lexer(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    lexer->source = strdup(source);
    if (!lexer->source) {
        free(lexer);
        return NULL;
    }

    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

void free_lexer(Lexer* lexer) {
    if (lexer) {
        if (lexer->source) free(lexer->source);
        free(lexer);
    }
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->position];
}

static char advance(Lexer* lexer) {
    char c = peek(lexer);
    lexer->position++;
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static char peek_next(Lexer* lexer) {
    if ((size_t)lexer->position + 1 >= strlen(lexer->source)) {
        return '\0';
    }
    return lexer->source[lexer->position + 1];
}

static void skip_whitespace_and_comments(Lexer* lexer) {
    while (peek(lexer)) {
        if (isspace(peek(lexer))) {
            if (peek(lexer) == '\n') {
                lexer->line++;
                lexer->column = 0;
            }
            advance(lexer);
        }
        // Skip single-line comments
        else if (peek(lexer) == '/' && peek_next(lexer) == '/') {
            while (peek(lexer) && peek(lexer) != '\n') {
                advance(lexer);
            }
        }
        else {
            break;
        }
    }
}

static Token* read_identifier(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_col = lexer->column;
    
    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    
    int length = lexer->position - start_pos;
    char* value = (char*)malloc(length + 1);
    strncpy(value, &lexer->source[start_pos], length);
    value[length] = '\0';
    
    // Check for keywords
    TokenType type = TOKEN_IDENTIFIER;
    if (strcmp(value, "bikin") == 0) type = TOKEN_BIKIN;
    else if (strcmp(value, "fungsi_int") == 0) type = TOKEN_FUNGSI_INT;
    else if (strcmp(value, "fungsi_float") == 0) type = TOKEN_FUNGSI_FLOAT;
    else if (strcmp(value, "fungsi_bool") == 0) type = TOKEN_FUNGSI_BOOL;
    else if (strcmp(value, "fungsi_str") == 0) type = TOKEN_FUNGSI_STR;
    else if (strcmp(value, "int") == 0) type = TOKEN_TYPE_INT;
    else if (strcmp(value, "float") == 0) type = TOKEN_TYPE_FLOAT;
    else if (strcmp(value, "bool") == 0) type = TOKEN_TYPE_BOOL;
    else if (strcmp(value, "str") == 0) type = TOKEN_TYPE_STR;
    else if (strcmp(value, "cetak") == 0) type = TOKEN_CETAK;
    else if (strcmp(value, "kalo") == 0) type = TOKEN_KALO;
    else if (strcmp(value, "maka") == 0) type = TOKEN_MAKA;
    else if (strcmp(value, "lain") == 0) type = TOKEN_LAIN;
    else if (strcmp(value, "ulang") == 0) type = TOKEN_ULANG;
    else if (strcmp(value, "sampai") == 0) type = TOKEN_SAMPAI;
    else if (strcmp(value, "fungsi") == 0) type = TOKEN_FUNGSI;
    else if (strcmp(value, "kembali") == 0) type = TOKEN_KEMBALI;
    else if (strcmp(value, "isi") == 0) type = TOKEN_ISI;
    else if (strcmp(value, "benar") == 0) type = TOKEN_BENAR;
    else if (strcmp(value, "salah") == 0) type = TOKEN_SALAH;
    else if (strcmp(value, "array") == 0) type = TOKEN_ARRAY;
    
    Token* token = create_token(type, value, lexer->line, start_col);
    free(value);
    return token;
}

static Token* read_number(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_col = lexer->column;
    bool is_float = false;
    
    while (isdigit(peek(lexer)) || peek(lexer) == '.') {
        if (peek(lexer) == '.') {
            if (is_float) break;  // Sudah ada titik desimal
            is_float = true;
        }
        advance(lexer);
    }
    
    int length = lexer->position - start_pos;
    char* value = (char*)malloc(length + 1);
    strncpy(value, &lexer->source[start_pos], length);
    value[length] = '\0';
    
    Token* token = create_token(
        is_float ? TOKEN_FLOAT : TOKEN_NUMBER,
        value,
        lexer->line,
        start_col
    );
    free(value);
    return token;
}

Token* get_next_token(Lexer* lexer) {
    if (!lexer || !lexer->source) {
        return NULL;
    }

    size_t source_len = strlen(lexer->source);
    if ((size_t)lexer->position >= source_len) {
        return NULL;
    }

    skip_whitespace_and_comments(lexer);
    
    char c = peek(lexer);
    int current_line = lexer->line;
    int current_col = lexer->column;
    
    // End of file
    if (c == '\0') {
        return NULL;
    }
    
    // Identifiers dan keywords
    if (isalpha(c) || c == '_') {
        return read_identifier(lexer);
    }
    
    // Numbers (integer dan float)
    if (isdigit(c)) {
        return read_number(lexer);
    }
    
    // String literals
    if (c == '"') {
        advance(lexer); // Skip opening quote
        int start_pos = lexer->position;
        int start_col = lexer->column;
        
        while (peek(lexer) != '"' && peek(lexer) != '\0') {
            advance(lexer);
        }
        
        if (peek(lexer) == '"') {
            int length = lexer->position - start_pos;
            char* value = (char*)malloc(length + 1);
            strncpy(value, &lexer->source[start_pos], length);
            value[length] = '\0';
            advance(lexer); // Skip closing quote
            
            Token* token = create_token(TOKEN_STRING, value, current_line, start_col);
            free(value);
            return token;
        }
        return NULL;  // Unterminated string
    }
    
    // Single character tokens
    advance(lexer);
    switch (c) {
        case '+': return create_token(TOKEN_PLUS, "+", current_line, current_col);
        case '-': return create_token(TOKEN_MINUS, "-", current_line, current_col);
        case '*': return create_token(TOKEN_MULTIPLY, "*", current_line, current_col);
        case '/': return create_token(TOKEN_DIVIDE, "/", current_line, current_col);
        case '=': return create_token(TOKEN_EQUALS, "=", current_line, current_col);
        case '<': return create_token(TOKEN_LESS, "<", current_line, current_col);
        case '>': return create_token(TOKEN_GREATER, ">", current_line, current_col);
        case '(': return create_token(TOKEN_LPAREN, "(", current_line, current_col);
        case ')': return create_token(TOKEN_RPAREN, ")", current_line, current_col);
        case '{': return create_token(TOKEN_LBRACE, "{", current_line, current_col);
        case '}': return create_token(TOKEN_RBRACE, "}", current_line, current_col);
        case '[': return create_token(TOKEN_LBRACKET, "[", current_line, current_col);
        case ']': return create_token(TOKEN_RBRACKET, "]", current_line, current_col);
        case ';': return create_token(TOKEN_SEMICOLON, ";", current_line, current_col);
        case ',': return create_token(TOKEN_COMMA, ",", current_line, current_col);
        case '.': return create_token(TOKEN_DOT, ".", current_line, current_col);
        case ':': return create_token(TOKEN_COLON, ":", current_line, current_col);
    }
    
    // Skip unknown character and continue
    return NULL;
}
