#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Penggunaan: azeng <file.az>\n");
        return 1;
    }

    // Baca file source
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Tidak bisa membuka file %s\n", argv[1]);
        return 1;
    }

    // Baca seluruh isi file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = (char*)malloc(file_size + 1);
    if (!source) {
        printf("Error: Gagal mengalokasi memori\n");
        fclose(file);
        return 1;
    }

    size_t bytes_read = fread(source, 1, file_size, file);
    source[bytes_read] = '\0';
    fclose(file);

    // Inisialisasi lexer
    Lexer* lexer = create_lexer(source);
    if (!lexer) {
        printf("Error: Gagal membuat lexer\n");
        free(source);
        return 1;
    }

    Parser* parser = create_parser(lexer);
    if (!parser) {
        printf("Error: Gagal membuat parser\n");
        free_lexer(lexer);
        free(source);
        return 1;
    }

    ASTNode* ast = parse(parser);
    if (ast) {
        Interpreter* interpreter = create_interpreter();
        if (interpreter) {
            interpret(interpreter, ast);
            free_interpreter(interpreter);
        }
        
        free_ast(ast);
    }

    free_parser(parser);
    free_lexer(lexer);
    free(source);

    return 0;
}
