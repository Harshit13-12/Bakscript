#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_NUM:
        return "NUM";
    case TOKEN_STR:
        return "STR";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_SHOW:
        return "SHOW";
    case TOKEN_WHEN:
        return "WHEN";
    case TOKEN_OTHERWISE:
        return "OTHERWISE";
    case TOKEN_REPEAT:
        return "REPEAT";
    case TOKEN_ASK:
        return "ASK";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_NUMBER_LITERAL:
        return "NUMBER_LITERAL";
    case TOKEN_STRING_LITERAL:
        return "STRING_LITERAL";
    case TOKEN_EQUALS:
        return "EQUALS";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_LBRACE:
        return "LBRACE";
    case TOKEN_RBRACE:
        return "RBRACE";
    case TOKEN_EOF:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

Parser *create_parser(Lexer *lexer)
{
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);
    return parser;
}

void parser_advance(Parser *parser)
{
    parser->current_token = lexer_get_next_token(parser->lexer);
}

bool parser_expect(Parser *parser, TokenType type)
{
    if (parser->current_token->type == type)
    {
        parser_advance(parser);
        return true;
    }
    else
    {
        fprintf(stderr, "Error: Expected %s but got '%s' (%s) at line %d, column %d\n",
                token_type_to_string(type),
                parser->current_token->value ? parser->current_token->value : "EOF",
                token_type_to_string(parser->current_token->type),
                parser->current_token->line,
                parser->current_token->column);
        return false;
    }
}

void free_parser(Parser *parser)
{
    free(parser->current_token);
    free(parser);
}

// Forward declarations for recursive descent
static Node *parse_block(Parser *parser);
Node *parse_primary(Parser *parser)
{
    Token *token = parser->current_token;

    switch (token->type)
    {
    case TOKEN_NUMBER_LITERAL:
        parser_advance(parser);
        return create_number_node(atoi(token->value), token->line, token->column);

    case TOKEN_STRING_LITERAL:
        parser_advance(parser);
        return create_string_node(token->value, token->line, token->column);

    case TOKEN_IDENTIFIER:
        parser_advance(parser);
        if (parser->current_token->type == TOKEN_LPAREN)
        {
            return parse_function_call(parser, token);
        }
        return create_identifier_node(token->value, token->line, token->column);

    case TOKEN_SHOW:
    case TOKEN_ASK:
        return parse_function_call(parser, token);

    case TOKEN_LPAREN:
        parser_advance(parser);
        Node *expr = parse_expression(parser);
        if (!expr)
            return NULL;
        if (!parser_expect(parser, TOKEN_RPAREN))
        {
            free_node(expr);
            return NULL;
        }
        return expr;

    default:
        return NULL;
    }
}

// Parse a factor (unary operations and primary expressions)
Node *parse_factor(Parser *parser)
{
    return parse_primary(parser);
}

// Parse a term (multiplication and division)
Node *parse_term(Parser *parser)
{
    Node *left = parse_primary(parser);
    if (!left)
        return NULL;

    while (parser->current_token->type == TOKEN_MULTIPLY ||
           parser->current_token->type == TOKEN_DIVIDE)
    {
        Token *op_token = parser->current_token;
        BinaryOpType op = (op_token->type == TOKEN_MULTIPLY) ? OP_MULTIPLY : OP_DIVIDE;
        parser_advance(parser);

        Node *right = parse_primary(parser);
        if (!right)
            return NULL;

        left = create_binary_op_node(op, left, right, op_token->line, op_token->column);
    }

    return left;
}

// Parse an expression (addition, subtraction, comparison)
Node *parse_expression(Parser *parser)
{
    Node *left = parse_term(parser);
    if (!left)
        return NULL;

    while (parser->current_token->type == TOKEN_PLUS ||
           parser->current_token->type == TOKEN_MINUS ||
           parser->current_token->type == TOKEN_LESS ||
           parser->current_token->type == TOKEN_GREATER)
    {
        Token *op_token = parser->current_token;
        BinaryOpType op;
        switch (op_token->type)
        {
        case TOKEN_PLUS:
            op = OP_ADD;
            break;
        case TOKEN_MINUS:
            op = OP_SUBTRACT;
            break;
        case TOKEN_LESS:
            op = OP_LESS;
            break;
        case TOKEN_GREATER:
            op = OP_GREATER;
            break;
        default:
            return NULL;
        }
        parser_advance(parser);

        Node *right = parse_term(parser);
        if (!right)
            return NULL;

        left = create_binary_op_node(op, left, right, op_token->line, op_token->column);
    }

    return left;
}

// Parse a block of statements
static Node *parse_block(Parser *parser)
{
    Token *start_token = parser->current_token;
    if (!parser_expect(parser, TOKEN_LBRACE))
        return NULL;

    Node **statements = NULL;
    int count = 0;
    int capacity = 0;

    while (parser->current_token->type != TOKEN_RBRACE)
    {
        if (count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            statements = realloc(statements, capacity * sizeof(Node *));
        }

        Node *stmt = parse_statement(parser);
        if (!stmt)
        {
            free(statements);
            return NULL;
        }

        statements[count++] = stmt;
    }

    if (!parser_expect(parser, TOKEN_RBRACE))
    {
        free(statements);
        return NULL;
    }

    return create_block_node(statements, count, start_token->line, start_token->column);
}

// Parse a variable declaration
Node *parse_variable_declaration(Parser *parser)
{
    Token *type_token = parser->current_token;
    const char *type = type_token->value;
    parser_advance(parser);

    Token *name_token = parser->current_token;
    if (!parser_expect(parser, TOKEN_IDENTIFIER))
        return NULL;
    const char *name = name_token->value;

    Node *initializer = NULL;
    if (parser->current_token->type == TOKEN_EQUALS)
    {
        parser_advance(parser);
        initializer = parse_expression(parser);
        if (!initializer)
            return NULL;

        if (!parser_expect(parser, TOKEN_SEMICOLON))
            return NULL;

        return create_var_decl_node(type, name, initializer, type_token->line, type_token->column);
    }

    if (!parser_expect(parser, TOKEN_SEMICOLON))
        return NULL;

    return create_var_decl_node(type, name, initializer, type_token->line, type_token->column);
}

// Parse an if statement
Node *parse_if_statement(Parser *parser)
{
    Token *if_token = parser->current_token;
    parser_advance(parser); 

    if (!parser_expect(parser, TOKEN_LPAREN))
        return NULL;

    Node *condition = parse_expression(parser);
    if (!condition)
        return NULL;

    if (!parser_expect(parser, TOKEN_RPAREN))
        return NULL;

    Node *if_body = parse_block(parser);
    if (!if_body)
        return NULL;

    Node *else_body = NULL;
    if (parser->current_token->type == TOKEN_OTHERWISE)
    {
        parser_advance(parser);
        else_body = parse_block(parser);
        if (!else_body)
            return NULL;
    }

    return create_if_node(condition, if_body, else_body, if_token->line, if_token->column);
}

// Parse an assignment expression (for loop increment)
Node *parse_assignment_expression(Parser *parser)
{
    Token *id_token = parser->current_token;
    if (id_token->type != TOKEN_IDENTIFIER)
    {
        return parse_expression(parser);
    }

    Node *left = create_identifier_node(id_token->value, id_token->line, id_token->column);
    parser_advance(parser);

    if (parser->current_token->type == TOKEN_EQUALS)
    {
        Token *op_token = parser->current_token;
        parser_advance(parser);
        Node *right = parse_expression(parser);
        if (!right)
            return NULL;

        return create_binary_op_node(OP_ASSIGN, left, right, op_token->line, op_token->column);
    }

    return left;
}

// Parse a for loop
Node *parse_for_loop(Parser *parser)
{
    Token *for_token = parser->current_token;
    parser_advance(parser); 

    if (!parser_expect(parser, TOKEN_LPAREN))
        return NULL;

    Node *initializer = parse_variable_declaration(parser);
    if (!initializer)
        return NULL;

    Node *condition = parse_expression(parser);
    if (!condition)
        return NULL;

    if (!parser_expect(parser, TOKEN_SEMICOLON))
        return NULL;
    Node *increment = parse_assignment_expression(parser);
    if (!increment)
        return NULL;

    if (!parser_expect(parser, TOKEN_RPAREN))
        return NULL;

    Node *body = parse_block(parser);
    if (!body)
        return NULL;

    return create_for_node(initializer, condition, increment, body, for_token->line, for_token->column);
}

// Parse a function call (show or ask)
Node *parse_function_call(Parser *parser, Token *func_token)
{
    parser_advance(parser);
    if (!parser_expect(parser, TOKEN_LPAREN))
        return NULL;

    Node **args = NULL;
    int arg_count = 0;

    if (parser->current_token->type != TOKEN_RPAREN)
    {
        args = malloc(sizeof(Node *));
        args[0] = parse_expression(parser);
        if (!args[0])
        {
            free(args);
            return NULL;
        }
        arg_count = 1;
    }

    if (!parser_expect(parser, TOKEN_RPAREN))
    {
        if (args)
            free(args);
        return NULL;
    }

    return create_function_call_node(func_token->value, args, arg_count, func_token->line, func_token->column);
}

// Parse a statement
Node *parse_statement(Parser *parser)
{
    switch (parser->current_token->type)
    {
    case TOKEN_NUM:
    case TOKEN_STR:
        return parse_variable_declaration(parser);

    case TOKEN_WHEN:
        return parse_if_statement(parser);

    case TOKEN_REPEAT:
        return parse_for_loop(parser);

    case TOKEN_SHOW:
    case TOKEN_ASK:
    {
        Node *func_call = parse_function_call(parser, parser->current_token);
        if (!parser_expect(parser, TOKEN_SEMICOLON))
        {
            free_node(func_call);
            return NULL;
        }
        return func_call;
    }

    case TOKEN_IDENTIFIER:
    {
        Token *token = parser->current_token;
        parser_advance(parser);

        if (parser->current_token->type == TOKEN_LPAREN)
        {
            parser_advance(parser); 
            Node **args = NULL;
            int arg_count = 0;

            if (parser->current_token->type != TOKEN_RPAREN)
            {
                args = (Node **)malloc(sizeof(Node *));
                args[0] = parse_expression(parser);
                if (!args[0])
                {
                    free(args);
                    return NULL;
                }
                arg_count = 1;
            }

            if (!parser_expect(parser, TOKEN_RPAREN))
            {
                if (args)
                    free(args);
                return NULL;
            }

            Node *func_call = create_function_call_node(token->value, args, arg_count, token->line, token->column);
            if (!parser_expect(parser, TOKEN_SEMICOLON))
            {
                free_node(func_call);
                return NULL;
            }
            return func_call;
        }

        Node *left = create_identifier_node(token->value, token->line, token->column);

        if (parser->current_token->type == TOKEN_EQUALS)
        {
            Token *op_token = parser->current_token;
            parser_advance(parser);
            Node *right = parse_expression(parser);
            if (!right)
            {
                free_node(left);
                return NULL;
            }
            if (!parser_expect(parser, TOKEN_SEMICOLON))
            {
                free_node(left);
                free_node(right);
                return NULL;
            }
            return create_binary_op_node(OP_ASSIGN, left, right, op_token->line, op_token->column);
        }
        else
        {
            // Reject standalone identifiers with a more user-friendly message
            fprintf(stderr, "Error: Invalid statement at line %d, column %d.\n"
                            "The identifier '%s' must be used in a proper statement like:\n"
                            "  - Variable declaration: num %s = value;\n"
                            "  - Assignment: %s = value;\n"
                            "  - Function call: %s(value);\n",
                    token->line, token->column,
                    token->value, token->value, token->value, token->value);
            free_node(left);
            return NULL;
        }
    }

    default:
        fprintf(stderr, "Error: Unexpected token '%s' (%s) in statement at line %d, column %d\n",
                parser->current_token->value ? parser->current_token->value : "EOF",
                token_type_to_string(parser->current_token->type),
                parser->current_token->line,
                parser->current_token->column);
        return NULL;
    }
}

// Parse the entire program
Node *parse_program(Parser *parser)
{
    Node **statements = NULL;
    int count = 0;

    while (parser->current_token->type != TOKEN_EOF)
    {
        statements = (Node **)realloc(statements, (count + 1) * sizeof(Node *));
        Node *stmt = parse_statement(parser);
        if (!stmt)
        {
            for (int i = 0; i < count; i++)
            {
                free_node(statements[i]);
            }
            free(statements);
            return NULL;
        }
        statements[count++] = stmt;
    }

    return create_program_node(statements, count);
}