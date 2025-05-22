#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"
typedef enum
{
    ERROR_UNDEFINED_VARIABLE,
    ERROR_DUPLICATE_VARIABLE,
    ERROR_TYPE_MISMATCH,
    ERROR_UNINITIALIZED_VARIABLE,
    ERROR_INVALID_OPERATION
} SemanticErrorType;
typedef struct
{
    SemanticErrorType type;
    char *message;
    int line;
    int column;
} SemanticError;
typedef struct
{
    SymbolTable *symbol_table;
    SemanticError *errors;
    int error_count;
    int error_capacity;
} SemanticContext;
SemanticContext *create_semantic_context(void);
void free_semantic_context(SemanticContext *context);
bool analyze_program(SemanticContext *context, Node *program);
DataType get_expression_type(SemanticContext *context, Node *expr);
void add_semantic_error(SemanticContext *context, SemanticErrorType type,
                        const char *message, int line, int column);
const char *get_error_type_string(SemanticErrorType type);

#endif