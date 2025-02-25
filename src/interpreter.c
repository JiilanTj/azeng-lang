#include "interpreter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

// Forward declarations
static Value evaluate_expression(Interpreter* interpreter, ASTNode* node);
static void interpret_block(Interpreter* interpreter, ASTNode* node);
static void interpret_function(Interpreter* interpreter, ASTNode* node);

// Tambahkan di bagian atas file setelah includes
#define MAX_ARRAY_SIZE 1000

// Struktur untuk menyimpan response
struct ResponseData {
    char* data;
    size_t size;
};

// Callback untuk CURL
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct ResponseData* resp = (struct ResponseData*)userp;
    
    char* ptr = realloc(resp->data, resp->size + realsize + 1);
    if(!ptr) return 0;
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = 0;
    
    return realsize;
}

// Implementasi HTTP GET
Value evaluate_http_get(const char* url) {
    Value result = {0};
    CURL* curl = curl_easy_init();
    if(curl) {
        struct ResponseData resp = {0};
        resp.data = malloc(1);
        resp.size = 0;
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&resp);
        
        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            result.type = TYPE_STRING;
            result.value.str_val = strdup(resp.data);
        }
        
        free(resp.data);
        curl_easy_cleanup(curl);
    }
    return result;
}

// Implementasi HTTP POST
Value evaluate_http_post(const char* url, const char* data) {
    Value result = {0};
    CURL* curl = curl_easy_init();
    if(curl) {
        struct ResponseData resp = {0};
        resp.data = malloc(1);
        resp.size = 0;
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&resp);
        
        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            result.type = TYPE_STRING;
            result.value.str_val = strdup(resp.data);
        }
        
        free(resp.data);
        curl_easy_cleanup(curl);
    }
    return result;
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
            return interpreter->variables[i].value;
        }
    }
    fprintf(stderr, "Error: Variable '%s' not found\n", name);
    return NULL;
}

// Modifikasi set_variable untuk menerima Value*
static void set_variable(Interpreter* interpreter, const char* name, Value* value) {
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            free_value(interpreter->variables[i].value);
            interpreter->variables[i].value = value;
            return;
        }
    }
    
    if (interpreter->variable_count < MAX_VARIABLES) {
        interpreter->variables[interpreter->variable_count].name = strdup(name);
        interpreter->variables[interpreter->variable_count].value = value;
        interpreter->variable_count++;
    } else {
        fprintf(stderr, "Error: Too many variables\n");
        exit(1);
    }
}

// Fungsi helper untuk array
static Value* create_array_value(int size) {
    Value* val = malloc(sizeof(Value));
    if (!val) return NULL;
    
    val->type = TYPE_ARRAY_INT;
    val->array_size = size;
    val->value.int_array = calloc(size, sizeof(int));
    
    return val;
}

static void interpret_array_assign(Interpreter* interpreter, ASTNode* node) {
    if (!node || node->children_count != 2) return;
    
    Value* arr = get_variable(interpreter, node->value);
    if (!arr || arr->type != TYPE_ARRAY_INT) {
        fprintf(stderr, "Error: Invalid array access\n");
        return;
    }
    
    Value index_val = evaluate_expression(interpreter, node->children[0]);
    int index = index_val.value.int_val;
    
    if (index < 0 || index >= arr->array_size) {
        fprintf(stderr, "Error: Array index out of bounds\n");
        return;
    }
    
    Value new_val = evaluate_expression(interpreter, node->children[1]);
    arr->value.int_array[index] = new_val.value.int_val;
}

// Tambahkan fungsi untuk process escape sequences
static char* process_string(const char* input) {
    if (!input) return NULL;
    
    int len = strlen(input);
    char* output = malloc(len + 1);
    int j = 0;
    
    for (int i = 0; i < len; i++) {
        if (input[i] == '\\' && i + 1 < len) {
            switch (input[i + 1]) {
                case 'n':
                    output[j++] = '\n';
                    i++;
                    break;
                case 't':
                    output[j++] = '\t';
                    i++;
                    break;
                case '\\':
                    output[j++] = '\\';
                    i++;
                    break;
                default:
                    output[j++] = input[i];
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
    return output;
}

// Update fungsi evaluate_expression untuk string literals
static Value evaluate_expression(Interpreter* interpreter, ASTNode* node) {
    Value result = {0};
    if (!node) return result;

    switch (node->type) {
        case AST_NUMBER:
            result.type = TYPE_INT;
            result.value.int_val = atoi(node->value);
            break;
            
        case AST_FLOAT:
            result.type = TYPE_FLOAT;
            result.value.float_val = atof(node->value);
            break;
            
        case AST_STRING: {
            result.type = TYPE_STRING;
            result.value.str_val = process_string(node->value);
            break;
        }
            
        case AST_BOOLEAN:
            result.type = TYPE_BOOLEAN;
            result.value.bool_val = strcmp(node->value, "benar") == 0;
            break;
            
        case AST_IDENTIFIER: {
            Value* var = get_variable(interpreter, node->value);
            if (var) {
                result = *var;  // Copy the value
            }
            break;
        }
            
        case AST_CALL: {
            if (strcmp(node->value, "http_get") == 0) {
                Value url_val = evaluate_expression(interpreter, node->children[0]);
                if (url_val.type == TYPE_STRING) {
                    result = evaluate_http_get(url_val.value.str_val);
                    free(url_val.value.str_val);
                }
            }
            else if (strcmp(node->value, "http_post") == 0) {
                Value url_val = evaluate_expression(interpreter, node->children[0]);
                Value data_val = evaluate_expression(interpreter, node->children[1]);
                if (url_val.type == TYPE_STRING && data_val.type == TYPE_STRING) {
                    result = evaluate_http_post(url_val.value.str_val, data_val.value.str_val);
                    free(url_val.value.str_val);
                    free(data_val.value.str_val);
                }
            }
            else if (strcmp(node->value, "tambah") == 0) {
                if (node->children_count == 2) {
                    Value left = evaluate_expression(interpreter, node->children[0]);
                    Value right = evaluate_expression(interpreter, node->children[1]);
                    
                    if (left.type == TYPE_INT && right.type == TYPE_INT) {
                        result.type = TYPE_INT;
                        result.value.int_val = left.value.int_val + right.value.int_val;
                    }
                }
            }
            else if (strcmp(node->value, "bagi") == 0) {
                if (node->children_count == 2) {
                    Value left = evaluate_expression(interpreter, node->children[0]);
                    Value right = evaluate_expression(interpreter, node->children[1]);
                    
                    if (left.type == TYPE_FLOAT && right.type == TYPE_FLOAT) {
                        result.type = TYPE_FLOAT;
                        result.value.float_val = left.value.float_val / right.value.float_val;
                    }
                }
            }
            else if (strcmp(node->value, "lebih_besar") == 0) {
                if (node->children_count == 2) {
                    Value left = evaluate_expression(interpreter, node->children[0]);
                    Value right = evaluate_expression(interpreter, node->children[1]);
                    
                    if (left.type == TYPE_INT && right.type == TYPE_INT) {
                        result.type = TYPE_BOOLEAN;
                        result.value.bool_val = left.value.int_val > right.value.int_val;
                    }
                }
            }
            else if (strcmp(node->value, "gabung") == 0) {
                if (node->children_count == 2) {
                    Value left = evaluate_expression(interpreter, node->children[0]);
                    Value right = evaluate_expression(interpreter, node->children[1]);
                    
                    if (left.type == TYPE_STRING && right.type == TYPE_STRING) {
                        char* str_result = malloc(strlen(left.value.str_val) + strlen(right.value.str_val) + 1);
                        strcpy(str_result, left.value.str_val);
                        strcat(str_result, right.value.str_val);
                        
                        // Langsung set ke result
                        result.type = TYPE_STRING;
                        result.value.str_val = str_result;  // Transfer ownership
                        
                        // Cleanup input strings jika perlu
                        if (left.type == TYPE_STRING) free(left.value.str_val);
                        if (right.type == TYPE_STRING) free(right.value.str_val);
                    }
                }
            }
            
            return result;
        }
            
        case AST_BINARY_OP: {
            Value left = evaluate_expression(interpreter, node->children[0]);
            Value right = evaluate_expression(interpreter, node->children[1]);
            
            if (left.type == TYPE_INT && right.type == TYPE_INT) {
                result.type = TYPE_INT;
                if (strcmp(node->value, "+") == 0)
                    result.value.int_val = left.value.int_val + right.value.int_val;
                else if (strcmp(node->value, "-") == 0)
                    result.value.int_val = left.value.int_val - right.value.int_val;
                else if (strcmp(node->value, "*") == 0)
                    result.value.int_val = left.value.int_val * right.value.int_val;
                else if (strcmp(node->value, "/") == 0)
                    result.value.int_val = left.value.int_val / right.value.int_val;
                else if (strcmp(node->value, "<") == 0)
                    result.value.bool_val = left.value.int_val < right.value.int_val;
                else if (strcmp(node->value, ">") == 0)
                    result.value.bool_val = left.value.int_val > right.value.int_val;
            }
            else if (left.type == TYPE_FLOAT && right.type == TYPE_FLOAT) {
                result.type = TYPE_FLOAT;
                if (strcmp(node->value, "+") == 0)
                    result.value.float_val = left.value.float_val + right.value.float_val;
                else if (strcmp(node->value, "-") == 0)
                    result.value.float_val = left.value.float_val - right.value.float_val;
                else if (strcmp(node->value, "*") == 0)
                    result.value.float_val = left.value.float_val * right.value.float_val;
                else if (strcmp(node->value, "/") == 0)
                    result.value.float_val = left.value.float_val / right.value.float_val;
            }
            else if (left.type == TYPE_STRING && right.type == TYPE_STRING &&
                     strcmp(node->value, "+") == 0) {
                char* str_result = malloc(strlen(left.value.str_val) + strlen(right.value.str_val) + 1);
                strcpy(str_result, left.value.str_val);
                strcat(str_result, right.value.str_val);
                result.type = TYPE_STRING;
                result.value.str_val = str_result;
            }
            break;
        }
        
        case AST_ARRAY_DECL: {
            Value size_val = evaluate_expression(interpreter, node->children[0]);
            Value* array = create_array_value(size_val.value.int_val);
            if (array) {
                result = *array;
                free(array);
            }
            break;
        }
        
        case AST_ARRAY_ACCESS: {
            Value* arr = get_variable(interpreter, node->value);
            if (!arr || arr->type != TYPE_ARRAY_INT) {
                fprintf(stderr, "Error: Invalid array access\n");
                break;
            }
            
            Value index_val = evaluate_expression(interpreter, node->children[0]);
            int index = index_val.value.int_val;
            
            if (index < 0 || index >= arr->array_size) {
                fprintf(stderr, "Error: Array index out of bounds\n");
                break;
            }
            
            result.type = TYPE_INT;
            result.value.int_val = arr->value.int_array[index];
            break;
        }
        
        default:
            result.type = TYPE_VOID;
            break;
    }
    
    return result;
}

// Update fungsi untuk format JSON output
static void print_formatted_json(const char* json) {
    int indent = 0;
    int len = strlen(json);
    bool in_string = false;
    
    for (int i = 0; i < len; i++) {
        char c = json[i];
        
        // Handle string literals
        if (c == '"' && (i == 0 || json[i-1] != '\\')) {
            in_string = !in_string;
            printf("%c", c);
            continue;
        }
        
        if (!in_string) {
            switch (c) {
                case '{':
                case '[':
                    printf("%c\n", c);
                    indent += 2;
                    for (int j = 0; j < indent; j++) printf(" ");
                    break;
                    
                case '}':
                case ']':
                    printf("\n");
                    indent -= 2;
                    for (int j = 0; j < indent; j++) printf(" ");
                    printf("%c", c);
                    break;
                    
                case ',':
                    printf("%c\n", c);
                    for (int j = 0; j < indent; j++) printf(" ");
                    break;
                    
                case ':':
                    printf("%c ", c);
                    break;
                    
                default:
                    printf("%c", c);
            }
        } else {
            printf("%c", c);
        }
    }
    printf("\n");
}

// Update fungsi cetak untuk handle JSON
static Value evaluate_print(Value arg) {
    Value result = {0};
    result.type = TYPE_VOID;
    
    switch (arg.type) {
        case TYPE_STRING:
        case TYPE_HTTP_RESPONSE:
            if (arg.value.str_val && 
                (arg.value.str_val[0] == '{' || arg.value.str_val[0] == '[')) {
                print_formatted_json(arg.value.str_val);
            } else {
                printf("%s\n", arg.value.str_val ? arg.value.str_val : "");
            }
            break;
            
        case TYPE_INT:
            printf("%d\n", arg.value.int_val);
            break;
            
        case TYPE_FLOAT:
            printf("%f\n", arg.value.float_val);
            break;
            
        case TYPE_BOOLEAN:
            printf("%s\n", arg.value.bool_val ? "benar" : "salah");
            break;
            
        case TYPE_ARRAY_INT:
            printf("[");
            for (int i = 0; i < arg.array_size; i++) {
                printf("%d%s", arg.value.int_array[i], 
                       i < arg.array_size - 1 ? ", " : "");
            }
            printf("]\n");
            break;
            
        case TYPE_ARRAY_FLOAT:
            printf("[");
            for (int i = 0; i < arg.array_size; i++) {
                printf("%f%s", arg.value.float_array[i], 
                       i < arg.array_size - 1 ? ", " : "");
            }
            printf("]\n");
            break;
            
        case TYPE_ARRAY_BOOL:
            printf("[");
            for (int i = 0; i < arg.array_size; i++) {
                printf("%s%s", arg.value.bool_array[i] ? "benar" : "salah", 
                       i < arg.array_size - 1 ? ", " : "");
            }
            printf("]\n");
            break;
            
        case TYPE_ARRAY_STRING:
            printf("[");
            for (int i = 0; i < arg.array_size; i++) {
                printf("\"%s\"%s", arg.value.str_array[i] ? arg.value.str_array[i] : "", 
                       i < arg.array_size - 1 ? ", " : "");
            }
            printf("]\n");
            break;
            
        case TYPE_VOID:
            printf("void\n");
            break;
    }
    return result;
}

static void interpret_call(Interpreter* interpreter, ASTNode* node) {
    if (strcmp(node->value, "cetak") == 0 && node->children_count > 0) {
        Value expr_val = evaluate_expression(interpreter, node->children[0]);
        evaluate_print(expr_val);
        // Hanya free jika tipe data memerlukan free
        if (expr_val.type == TYPE_STRING || expr_val.type == TYPE_HTTP_RESPONSE) {
            free(expr_val.value.str_val);
        }
    } else if (strcmp(node->value, "http_get") == 0) {
        Value url_val = evaluate_expression(interpreter, node->children[0]);
        if (url_val.type == TYPE_STRING) {
            Value result = evaluate_http_get(url_val.value.str_val);
            evaluate_print(result);
            // Free string hasil HTTP request
            if (result.value.str_val) {
                free(result.value.str_val);
            }
            // Free URL
            if (url_val.value.str_val) {
                free(url_val.value.str_val);
            }
        }
    } else if (strcmp(node->value, "http_post") == 0) {
        Value url_val = evaluate_expression(interpreter, node->children[0]);
        Value data_val = evaluate_expression(interpreter, node->children[1]);
        if (url_val.type == TYPE_STRING && data_val.type == TYPE_STRING) {
            Value result = evaluate_http_post(url_val.value.str_val, data_val.value.str_val);
            evaluate_print(result);
            // Free string hasil HTTP request
            if (result.value.str_val) {
                free(result.value.str_val);
            }
            // Free URL dan data
            if (url_val.value.str_val) {
                free(url_val.value.str_val);
            }
            if (data_val.value.str_val) {
                free(data_val.value.str_val);
            }
        }
    }
}

// Update fungsi interpret_variable_decl
static void interpret_variable_decl(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        Value expr_val = evaluate_expression(interpreter, node->children[0]);
        Value* value = malloc(sizeof(Value));
        *value = expr_val;  // Copy value
        set_variable(interpreter, node->value, value);
    }
}

// Update fungsi interpret_assignment
static void interpret_assignment(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count > 0) {
        Value expr_val = evaluate_expression(interpreter, node->children[0]);
        Value* value = malloc(sizeof(Value));
        *value = expr_val;  // Copy value
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
        
        if (evaluate_expression(interpreter, condition).value.bool_val) {
            interpret(interpreter, body);
        }
    }
}

static void interpret_while(Interpreter* interpreter, ASTNode* node) {
    if (node->children_count >= 2) {  // Harus punya kondisi dan body
        while (1) {
            Value condition = evaluate_expression(interpreter, node->children[0]);
            int continue_loop = condition.value.bool_val;
            
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
            
        case AST_ARRAY_ASSIGN:
            interpret_array_assign(interpreter, node);
            break;
            
        default:
            break;
    }
}

Value* create_array(DataType type, int size) {
    Value* arr = malloc(sizeof(Value));
    arr->type = type;
    arr->array_size = size;
    
    switch(type) {
        case TYPE_ARRAY_INT:
            arr->value.int_array = calloc(size, sizeof(int));
            break;
        case TYPE_ARRAY_FLOAT:
            arr->value.float_array = calloc(size, sizeof(float));
            break;
        case TYPE_ARRAY_BOOL:
            arr->value.bool_array = calloc(size, sizeof(int));
            break;
        case TYPE_ARRAY_STRING:
            arr->value.str_array = calloc(size, sizeof(char*));
            break;
        default:
            free(arr);
            return NULL;
    }
    return arr;
}

// Fungsi untuk mengakses array
Value* array_get(Value* arr, int index) {
    if (!arr || index < 0 || index >= arr->array_size) {
        return NULL;
    }
    
    Value* result = malloc(sizeof(Value));
    result->type = arr->type;
    result->array_size = 0;
    
    switch(arr->type) {
        case TYPE_ARRAY_INT:
            result->type = TYPE_INT;
            result->value.int_val = arr->value.int_array[index];
            break;
        case TYPE_ARRAY_FLOAT:
            result->type = TYPE_FLOAT;
            result->value.float_val = arr->value.float_array[index];
            break;
        case TYPE_ARRAY_BOOL:
            result->type = TYPE_BOOLEAN;
            result->value.bool_val = arr->value.bool_array[index];
            break;
        case TYPE_ARRAY_STRING:
            result->type = TYPE_STRING;
            result->value.str_val = strdup(arr->value.str_array[index] ? 
                                         arr->value.str_array[index] : "");
            break;
        default:
            free(result);
            return NULL;
    }
    return result;
}

// Fungsi untuk mengubah nilai array
bool array_set(Value* arr, int index, Value* value) {
    if (!arr || !value || index < 0 || index >= arr->array_size) {
        return false;
    }
    
    switch(arr->type) {
        case TYPE_ARRAY_INT:
            if (value->type != TYPE_INT) return false;
            arr->value.int_array[index] = value->value.int_val;
            break;
        case TYPE_ARRAY_FLOAT:
            if (value->type != TYPE_FLOAT) return false;
            arr->value.float_array[index] = value->value.float_val;
            break;
        case TYPE_ARRAY_BOOL:
            if (value->type != TYPE_BOOLEAN) return false;
            arr->value.bool_array[index] = value->value.bool_val;
            break;
        case TYPE_ARRAY_STRING:
            if (value->type != TYPE_STRING) return false;
            if (arr->value.str_array[index]) {
                free(arr->value.str_array[index]);
            }
            arr->value.str_array[index] = strdup(value->value.str_val);
            break;
        default:
            return false;
    }
    return true;
}

void free_value(Value* value) {
    if (!value) return;
    
    switch(value->type) {
        case TYPE_STRING:
        case TYPE_HTTP_RESPONSE:  // Handle sama seperti string
            if (value->value.str_val) free(value->value.str_val);
            break;
        case TYPE_ARRAY_INT:
            if (value->value.int_array) free(value->value.int_array);
            break;
        case TYPE_ARRAY_FLOAT:
            if (value->value.float_array) free(value->value.float_array);
            break;
        case TYPE_ARRAY_STRING:
            if (value->value.str_array) {
                for (int i = 0; i < value->array_size; i++) {
                    if (value->value.str_array[i]) {
                        free(value->value.str_array[i]);
                    }
                }
                free(value->value.str_array);
            }
            break;
        case TYPE_ARRAY_BOOL:
            if (value->value.bool_array) free(value->value.bool_array);
            break;
        // Primitive types don't need cleanup
        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_BOOLEAN:
        case TYPE_VOID:
            break;
    }
    free(value);
} 