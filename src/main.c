#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/tac.h"
#include "../include/gen.h"

void print_token(Token *token)
{
    const char *type_names[] = {
        "TOKEN_NUM", "TOKEN_STR",
        "TOKEN_PLUS", "TOKEN_MINUS", "TOKEN_MULTIPLY", "TOKEN_DIVIDE",
        "TOKEN_LESS", "TOKEN_GREATER",
        "TOKEN_SHOW", "TOKEN_WHEN", "TOKEN_OTHERWISE", "TOKEN_REPEAT", "TOKEN_ASK",
        "TOKEN_IDENTIFIER", "TOKEN_NUMBER_LITERAL", "TOKEN_STRING_LITERAL",
        "TOKEN_EQUALS", "TOKEN_SEMICOLON",
        "TOKEN_LPAREN", "TOKEN_RPAREN", "TOKEN_LBRACE", "TOKEN_RBRACE",
        "TOKEN_EOF"};
    printf("Token(type=%s, value='%s', line=%d, column=%d)\n",
           type_names[token->type], token->value ? token->value : "", token->line, token->column);
}

void print_ast(Node *node, int indent)
{
    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (node->type)
    {
    case NODE_NUMBER:
        printf("Number: %d\n", node->number.value);
        break;

    case NODE_STRING:
        printf("String: \"%s\"\n", node->string.value);
        break;

    case NODE_IDENTIFIER:
        printf("Identifier: %s\n", node->identifier.name);
        break;

    case NODE_BINARY_OP:
        printf("BinaryOp: %d\n", node->binary_op.op);
        print_ast(node->binary_op.left, indent + 1);
        print_ast(node->binary_op.right, indent + 1);
        break;

    case NODE_FUNCTION_CALL:
        printf("FunctionCall: %s\n", node->function_call.name);
        for (int i = 0; i < node->function_call.arg_count; i++)
        {
            print_ast(node->function_call.arguments[i], indent + 1);
        }
        break;

    case NODE_VARIABLE_DECLARATION:
        printf("VarDecl: %s %s\n", node->var_decl.type, node->var_decl.name);
        print_ast(node->var_decl.initializer, indent + 1);
        break;

    case NODE_IF_STATEMENT:
        printf("IfStatement:\n");
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Condition:\n");
        print_ast(node->if_stmt.condition, indent + 1);
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Then:\n");
        print_ast(node->if_stmt.if_body, indent + 1);
        if (node->if_stmt.else_body)
        {
            for (int i = 0; i < indent; i++)
                printf("  ");
            printf("Else:\n");
            print_ast(node->if_stmt.else_body, indent + 1);
        }
        break;

    case NODE_FOR_LOOP:
        printf("ForLoop:\n");
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Init:\n");
        print_ast(node->for_loop.initializer, indent + 1);
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Condition:\n");
        print_ast(node->for_loop.condition, indent + 1);
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Increment:\n");
        print_ast(node->for_loop.increment, indent + 1);
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Body:\n");
        print_ast(node->for_loop.body, indent + 1);
        break;

    case NODE_BLOCK:
    case NODE_PROGRAM:
        printf("%s:\n", node->type == NODE_BLOCK ? "Block" : "Program");
        for (int i = 0; i < node->block.count; i++)
        {
            print_ast(node->block.statements[i], indent + 1);
        }
        break;
    }
}

// FILE READ
char *read_file(const char *filename)
{
    FILE *file;
    if (filename)
    {
        file = fopen(filename, "r");
        if (!file)
        {
            fprintf(stderr, "Error: Could not open file '%s'\n", filename);
            exit(1);
        }
    }
    else
    {
        file = stdin;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer)
    {
        fprintf(stderr, "Error: Could not allocate memory for file contents\n");
        if (filename)
            fclose(file);
        exit(1);
    }

    // Read file
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';

    if (filename)
        fclose(file);
    return buffer;
}

int main(int argc, char *argv[])
{
    char *source;
    const char *filename = NULL;

    if (argc > 1)
    {
        filename = argv[1];
    }

    source = read_file(filename);

    if (filename)
    {
        printf("Processing file: %s\n\n", filename);
    }
    printf("Source code:\n%s\n", source);

    printf("\nTokens:\n");
    Lexer *lexer = create_lexer(source);
    Token *token;
    TokenType type;

    do
    {
        token = lexer_get_next_token(lexer);
        type = token->type;
        print_token(token);
        free(token->value);
        free(token);
    } while (type != TOKEN_EOF);

    free_lexer(lexer);
    lexer = create_lexer(source);
    Parser *parser = create_parser(lexer);

    printf("\nParsing.....\n");
    Node *ast = parse_program(parser);

    if (ast != NULL)
    {
        printf("\nAbstract Syntax Tree:\n");
        print_ast(ast, 0);

        printf("\nPerforming semantic analysis.....\n");
        SemanticContext *context = create_semantic_context();
        bool success = analyze_program(context, ast);

        if (!success)
        {
            printf("\nSemantic errors found:\n");
            for (int i = 0; i < context->error_count; i++)
            {
                SemanticError *error = &context->errors[i];
                printf("Error at line %d, column %d: %s - %s\n",
                       error->line, error->column,
                       get_error_type_string(error->type),
                       error->message);
            }
        }
        else
        {
            printf("\nNo semantic errors found.\n");
            printf("\nGenerating Three-Address Code.....\n");
            TAC *tac = ast_to_tac(ast);
            if (tac)
            {
                printf("\nGenerated Three-Address Code:\n");
                printf("----------------------------\n");
                print_tac_list(tac);
                printf("\nGenerating Assembly Code.....\n");
                char *assembly = generate_code(tac);
                if (assembly)
                {
                    const char *prefix = "default rel\n\n";
                    size_t prefix_len = strlen(prefix);
                    size_t assembly_len = strlen(assembly);
                    // Allocate new buffer for combined string
                    char *new_assembly = malloc(prefix_len + assembly_len + 1);
                    if (!new_assembly)
                    {
                        printf("Error: Memory allocation failed\n");
                        free(assembly);
                        tac_free(tac);
                        free_semantic_context(context);
                        free_node(ast);
                        free_parser(parser);
                        free_lexer(lexer);
                        free(source);
                        return 1;
                    }
                    memcpy(new_assembly, prefix, prefix_len);
                    memcpy(new_assembly + prefix_len, assembly, assembly_len + 1);

                    printf("\nGenerated Assembly Code:\n");
                    printf("----------------------------\n");
                    printf("%s\n", new_assembly);
                    FILE *fout = fopen("x86_64.asm", "w");
                    if (fout)
                    {
                        fputs(new_assembly, fout);
                        fclose(fout);
                        printf("Assembly code written to x86_64.asm\n");
                    }
                    else
                    {
                        printf("Error: Could not write to x86_64.asm\n");
                    }

                    free(new_assembly);
                    free(assembly);
                }

                else
                {
                    printf("Error: Failed to generate assembly code\n");
                }

                tac_free(tac);
            }
            else
            {
                printf("Error: Failed to generate TAC\n");
            }
        }

        free_semantic_context(context);
        free_node(ast);
    }
    else
    {
        printf("\nError: Failed to parse the program\n");
    }

    free_parser(parser);
    free_lexer(lexer);
    free(source);

    return 0;
}