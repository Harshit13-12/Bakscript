#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
typedef enum
{
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION
} SymbolType;

typedef enum
{
    TYPE_NUM,
    TYPE_STR,
    TYPE_VOID
} DataType;
typedef struct Symbol
{
    char *name;
    SymbolType symbol_type;
    DataType data_type;
    bool is_initialized;
    int scope_level;
    struct Symbol *next; 
} Symbol;

typedef struct SymbolTable
{
    Symbol **symbols; 
    int size;         
    int scope_level;  
} SymbolTable;

SymbolTable *create_symbol_table(int size);
void free_symbol_table(SymbolTable *table);
bool symbol_table_insert(SymbolTable *table, const char *name, SymbolType sym_type, DataType data_type);
Symbol *symbol_table_lookup(SymbolTable *table, const char *name);
void symbol_table_enter_scope(SymbolTable *table);
void symbol_table_exit_scope(SymbolTable *table);
void symbol_table_set_initialized(SymbolTable *table, const char *name);
void print_symbol_table(SymbolTable *table);

#endif 