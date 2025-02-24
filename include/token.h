#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    
    // Operators
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_MULTIPLY,  // *
    TOKEN_DIVIDE,    // /
    TOKEN_EQUALS,    // =
    TOKEN_LESS,      // <
    TOKEN_GREATER,   // >
    
    // Delimiters
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    TOKEN_DOT,       // .
    TOKEN_COLON,     // :
    
    // Keywords Bahasa Indonesia
    TOKEN_BIKIN,     // bikin
    TOKEN_CETAK,     // cetak
    TOKEN_KALO,      // kalo
    TOKEN_MAKA,      // maka
    TOKEN_LAIN,      // lain
    TOKEN_ULANG,     // ulang
    TOKEN_SAMPAI,    // sampai
    TOKEN_FUNGSI,    // fungsi
    TOKEN_KEMBALI,   // kembali
    TOKEN_ISI,        // isi
    TOKEN_FLOAT,
    TOKEN_BOOLEAN,
    TOKEN_RETURN,
    TOKEN_FUNGSI_INT,    // fungsi_int
    TOKEN_FUNGSI_FLOAT,  // fungsi_float
    TOKEN_FUNGSI_BOOL,   // fungsi_bool
    TOKEN_FUNGSI_STR,    // fungsi_str
    TOKEN_BENAR,         // true
    TOKEN_SALAH,         // false
    TOKEN_TYPE_INT,    // Tambahkan ini untuk "int"
    TOKEN_TYPE_FLOAT,  // Tambahkan ini untuk "float"
    TOKEN_TYPE_BOOL,   // Tambahkan ini untuk "bool"
    TOKEN_TYPE_STR,    // Tambahkan ini untuk "str"
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

Token* create_token(TokenType type, const char* value, int line, int column);
void free_token(Token* token);

#endif
