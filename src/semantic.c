#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/semantic.h"

#define INITIAL_ERROR_CAPACITY 16

SemanticContext *create_semantic_context(void)
{
    SemanticContext *context = (SemanticContext *)malloc(sizeof(SemanticContext));
    context->symbol_table = create_symbol_table(256); // reasonable size for hash table
    context->errors = (SemanticError *)malloc(INITIAL_ERROR_CAPACITY * sizeof(SemanticError));
    context->error_count = 0;
    context->error_capacity = INITIAL_ERROR_CAPACITY;
    return context;
}

void add_semantic_error(SemanticContext *context, SemanticErrorType type,
                        const char *message, int line, int column)
{
    if (context->error_count >= context->error_capacity)
    {
        context->error_capacity *= 2;
        context->errors = (SemanticError *)realloc(context->errors,
                                                   context->error_capacity * sizeof(SemanticError));
    }

    SemanticError *error = &context->errors[context->error_count++];
    error->type = type;
    error->message = strdup(message);
    error->line = line;
    error->column = column;
}

const char *get_error_type_string(SemanticErrorType type)
{
    switch (type)
    {
    case ERROR_UNDEFINED_VARIABLE:
        return "Undefined variable";
    case ERROR_DUPLICATE_VARIABLE:
        return "Duplicate variable declaration";
    case ERROR_TYPE_MISMATCH:
        return "Type mismatch";
    case ERROR_UNINITIALIZED_VARIABLE:
        return "Use of uninitialized variable";
    case ERROR_INVALID_OPERATION:
        return "Invalid operation";
    case ERROR_DIVISION_BY_ZERO:
        return "Division by zero";
    default:
        return "Unknown error";
    }
}

static DataType analyze_binary_op(SemanticContext *context, Node *node)
{
    DataType left_type = get_expression_type(context, node->binary_op.left);
    DataType right_type = get_expression_type(context, node->binary_op.right);

    switch (node->binary_op.op)
    {
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
        if (left_type != TYPE_NUM || right_type != TYPE_NUM)
        {
            add_semantic_error(context, ERROR_TYPE_MISMATCH,
                               "Arithmetic operations require numeric operands",
                               node->binary_op.info.line,
                               node->binary_op.info.column);
            return TYPE_NUM; // Return NUM to continue analysis
        }
        if (node->binary_op.right->type == NODE_NUMBER)
        {
            // Assuming the number value is stored as a string, convert and check
            double divisor = (double)node->binary_op.right->number.value;
            if (divisor == 0.0)
            {
                add_semantic_error(context, ERROR_INVALID_OPERATION,
                                   "Division by zero detected",
                                   node->binary_op.info.line,
                                   node->binary_op.info.column);
            }
        }
        return TYPE_NUM;

    case OP_LESS:
    case OP_GREATER:
        if (left_type != right_type)
        {
            add_semantic_error(context, ERROR_TYPE_MISMATCH,
                               "Comparison operators require operands of the same type",
                               node->binary_op.info.line,
                               node->binary_op.info.column);
        }
        return TYPE_NUM; // Boolean result

    case OP_ASSIGN:
        if (left_type != right_type)
        {
            add_semantic_error(context, ERROR_TYPE_MISMATCH,
                               "Cannot assign value of different type",
                               node->binary_op.info.line,
                               node->binary_op.info.column);
        }
        return left_type;

    default:
        add_semantic_error(context, ERROR_INVALID_OPERATION,
                           "Unknown binary operator",
                           node->binary_op.info.line,
                           node->binary_op.info.column);
        return TYPE_NUM;
    }
}

static DataType get_function_return_type(const char *func_name)
{
    if (strcmp(func_name, "ask") == 0)
        return TYPE_STR;
    else if (strcmp(func_name, "show") == 0)
        return TYPE_VOID;
    return TYPE_VOID;
}

DataType get_expression_type(SemanticContext *context, Node *expr)
{
    if (!expr)
        return TYPE_VOID;

    switch (expr->type)
    {
    case NODE_NUMBER:
        return TYPE_NUM;

    case NODE_STRING:
        return TYPE_STR;

    case NODE_IDENTIFIER:
    {
        Symbol *symbol = symbol_table_lookup(context->symbol_table, expr->identifier.name);
        if (!symbol)
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Use of undefined variable '%s'", expr->identifier.name);
            add_semantic_error(context, ERROR_UNDEFINED_VARIABLE,
                               error_msg, expr->identifier.info.line, expr->identifier.info.column);
            return TYPE_VOID;
        }
        if (!symbol->is_initialized)
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Variable '%s' is used before being initialized", expr->identifier.name);
            add_semantic_error(context, ERROR_UNINITIALIZED_VARIABLE,
                               error_msg, expr->identifier.info.line, expr->identifier.info.column);
        }
        return symbol->data_type;
    }

    case NODE_BINARY_OP:
        return analyze_binary_op(context, expr);

    case NODE_FUNCTION_CALL:
        return get_function_return_type(expr->function_call.name);

    default:
        return TYPE_VOID;
    }
}

static void analyze_variable_declaration(SemanticContext *context, Node *node)
{
    DataType var_type = strcmp(node->var_decl.type, "num") == 0 ? TYPE_NUM : TYPE_STR;

    if (!symbol_table_insert(context->symbol_table, node->var_decl.name,
                             SYMBOL_VARIABLE, var_type))
    {
        add_semantic_error(context, ERROR_DUPLICATE_VARIABLE,
                           "Variable already declared in this scope",
                           node->var_decl.info.line,
                           node->var_decl.info.column);
        return;
    }

    if (node->var_decl.initializer)
    {
        DataType init_type = get_expression_type(context, node->var_decl.initializer);
        if (init_type != var_type && init_type != TYPE_VOID) // Allow void for function calls
        {
            add_semantic_error(context, ERROR_TYPE_MISMATCH,
                               "Initializer type does not match variable type",
                               node->var_decl.info.line,
                               node->var_decl.info.column);
        }
        else if (node->var_decl.initializer->type == NODE_FUNCTION_CALL)
        {
            // If initializing with a function call, mark as initialized
            symbol_table_set_initialized(context->symbol_table, node->var_decl.name);
        }
        else if (init_type == var_type)
        {
            symbol_table_set_initialized(context->symbol_table, node->var_decl.name);
        }
    }
}

static void analyze_if_statement(SemanticContext *context, Node *node)
{
    DataType cond_type = get_expression_type(context, node->if_stmt.condition);
    if (cond_type != TYPE_NUM)
    {
        add_semantic_error(context, ERROR_TYPE_MISMATCH,
                           "If condition must be a numeric expression",
                           node->if_stmt.info.line,
                           node->if_stmt.info.column);
    }

    symbol_table_enter_scope(context->symbol_table);
    analyze_program(context, node->if_stmt.if_body);
    symbol_table_exit_scope(context->symbol_table);

    if (node->if_stmt.else_body)
    {
        symbol_table_enter_scope(context->symbol_table);
        analyze_program(context, node->if_stmt.else_body);
        symbol_table_exit_scope(context->symbol_table);
    }
}

static void analyze_for_loop(SemanticContext *context, Node *node)
{
    symbol_table_enter_scope(context->symbol_table);

    if (node->for_loop.initializer)
    {
        analyze_program(context, node->for_loop.initializer);
    }

    if (node->for_loop.condition)
    {
        DataType cond_type = get_expression_type(context, node->for_loop.condition);
        if (cond_type != TYPE_NUM)
        {
            add_semantic_error(context, ERROR_TYPE_MISMATCH,
                               "For loop condition must be a numeric expression",
                               node->for_loop.info.line,
                               node->for_loop.info.column);
        }
    }

    if (node->for_loop.increment)
    {
        analyze_program(context, node->for_loop.increment);
    }

    analyze_program(context, node->for_loop.body);
    symbol_table_exit_scope(context->symbol_table);
}

static void analyze_function_call(SemanticContext *context, Node *node)
{
    // Check each argument
    for (int i = 0; i < node->function_call.arg_count; i++)
    {
        Node *arg = node->function_call.arguments[i];
        DataType arg_type = get_expression_type(context, arg);
    }
}

bool analyze_program(SemanticContext *context, Node *node)
{
    if (!node)
        return true;

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (int i = 0; i < node->block.count; i++)
        {
            analyze_program(context, node->block.statements[i]);
        }
        break;

    case NODE_VARIABLE_DECLARATION:
        analyze_variable_declaration(context, node);
        break;

    case NODE_IF_STATEMENT:
        analyze_if_statement(context, node);
        break;

    case NODE_FOR_LOOP:
        analyze_for_loop(context, node);
        break;

    case NODE_BINARY_OP:
        get_expression_type(context, node);
        break;

    case NODE_FUNCTION_CALL:
        analyze_function_call(context, node);
        break;

    default:
        break;
    }

    return context->error_count == 0;
}

void free_semantic_context(SemanticContext *context)
{
    if (!context)
        return;

    free_symbol_table(context->symbol_table);

    for (int i = 0; i < context->error_count; i++)
    {
        free(context->errors[i].message);
    }
    free(context->errors);
    free(context);
}