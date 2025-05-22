#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ast.h"

// Helper function to allocate a new node
static Node *create_node(NodeType type)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = type;
    return node;
}

Node *create_number_node(int value, int line, int column)
{
    Node *node = create_node(NODE_NUMBER);
    node->number.value = value;
    node->number.info.line = line;
    node->number.info.column = column;
    return node;
}

Node *create_string_node(const char *value, int line, int column)
{
    Node *node = create_node(NODE_STRING);
    node->string.value = strdup(value);
    node->string.info.line = line;
    node->string.info.column = column;
    return node;
}

Node *create_identifier_node(const char *name, int line, int column)
{
    Node *node = create_node(NODE_IDENTIFIER);
    node->identifier.name = strdup(name);
    node->identifier.info.line = line;
    node->identifier.info.column = column;
    return node;
}

Node *create_binary_op_node(BinaryOpType op, Node *left, Node *right, int line, int column)
{
    Node *node = create_node(NODE_BINARY_OP);
    node->binary_op.op = op;
    node->binary_op.left = left;
    node->binary_op.right = right;
    node->binary_op.info.line = line;
    node->binary_op.info.column = column;
    return node;
}

Node *create_function_call_node(const char *name, Node **arguments, int arg_count, int line, int column)
{
    Node *node = create_node(NODE_FUNCTION_CALL);
    node->function_call.name = strdup(name);
    node->function_call.arg_count = arg_count;
    node->function_call.info.line = line;
    node->function_call.info.column = column;

    if (arg_count > 0)
    {
        node->function_call.arguments = (Node **)malloc(sizeof(Node *) * arg_count);
        memcpy(node->function_call.arguments, arguments, sizeof(Node *) * arg_count);
    }
    else
    {
        node->function_call.arguments = NULL;
    }

    return node;
}

Node *create_var_decl_node(const char *type, const char *name, Node *initializer, int line, int column)
{
    Node *node = create_node(NODE_VARIABLE_DECLARATION);
    node->var_decl.type = strdup(type);
    node->var_decl.name = strdup(name);
    node->var_decl.initializer = initializer;
    node->var_decl.info.line = line;
    node->var_decl.info.column = column;
    return node;
}

Node *create_if_node(Node *condition, Node *if_body, Node *else_body, int line, int column)
{
    Node *node = create_node(NODE_IF_STATEMENT);
    node->if_stmt.condition = condition;
    node->if_stmt.if_body = if_body;
    node->if_stmt.else_body = else_body;
    node->if_stmt.info.line = line;
    node->if_stmt.info.column = column;
    return node;
}

Node *create_for_node(Node *initializer, Node *condition, Node *increment, Node *body, int line, int column)
{
    Node *node = create_node(NODE_FOR_LOOP);
    node->for_loop.initializer = initializer;
    node->for_loop.condition = condition;
    node->for_loop.increment = increment;
    node->for_loop.body = body;
    node->for_loop.info.line = line;
    node->for_loop.info.column = column;
    return node;
}

Node *create_block_node(Node **statements, int count, int line, int column)
{
    Node *node = create_node(NODE_BLOCK);
    node->block.count = count;
    node->block.info.line = line;
    node->block.info.column = column;

    if (count > 0)
    {
        node->block.statements = (Node **)malloc(sizeof(Node *) * count);
        memcpy(node->block.statements, statements, sizeof(Node *) * count);
    }
    else
    {
        node->block.statements = NULL;
    }

    return node;
}

Node *create_program_node(Node **statements, int count)
{
    Node *node = create_node(NODE_PROGRAM);
    node->program.count = count;
    node->program.info.line = 1; // Program starts at line 1
    node->program.info.column = 1;

    if (count > 0)
    {
        node->program.statements = (Node **)malloc(sizeof(Node *) * count);
        memcpy(node->program.statements, statements, sizeof(Node *) * count);
    }
    else
    {
        node->program.statements = NULL;
    }

    return node;
}

void free_node(Node *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_STRING:
        free(node->string.value);
        break;

    case NODE_IDENTIFIER:
        free(node->identifier.name);
        break;

    case NODE_BINARY_OP:
        free_node(node->binary_op.left);
        free_node(node->binary_op.right);
        break;

    case NODE_FUNCTION_CALL:
        free(node->function_call.name);
        for (int i = 0; i < node->function_call.arg_count; i++)
        {
            free_node(node->function_call.arguments[i]);
        }
        free(node->function_call.arguments);
        break;

    case NODE_VARIABLE_DECLARATION:
        free(node->var_decl.type);
        free(node->var_decl.name);
        free_node(node->var_decl.initializer);
        break;

    case NODE_IF_STATEMENT:
        free_node(node->if_stmt.condition);
        free_node(node->if_stmt.if_body);
        if (node->if_stmt.else_body)
        {
            free_node(node->if_stmt.else_body);
        }
        break;

    case NODE_FOR_LOOP:
        free_node(node->for_loop.initializer);
        free_node(node->for_loop.condition);
        free_node(node->for_loop.increment);
        free_node(node->for_loop.body);
        break;

    case NODE_BLOCK:
    case NODE_PROGRAM:
        for (int i = 0; i < node->block.count; i++)
        {
            free_node(node->block.statements[i]);
        }
        free(node->block.statements);
        break;

    default:
        break;
    }

    free(node);
}