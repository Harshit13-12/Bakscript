#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

Lexer *create_lexer(char *source)
{
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->source = strdup(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = lexer->source[0];
    return lexer;
}

void lexer_advance(Lexer *lexer)
{
    if (lexer->current_char != '\0')
    {
        lexer->position++;
        lexer->column++;
        if (lexer->current_char == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        lexer->current_char = lexer->source[lexer->position];
    }
}

void lexer_skip_whitespace(Lexer *lexer)
{
    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        lexer_advance(lexer);
    }
}

Token *create_token(TokenType type, const char *value, int line, int column)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->value = value ? strdup(value) : NULL;
    token->line = line;
    token->column = column;
    return token;
}

char *lexer_collect_string(Lexer *lexer)
{
    char *str = (char *)malloc(1024 * sizeof(char));
    int i = 0;

    lexer_advance(lexer);

    while (lexer->current_char != '"' && lexer->current_char != '\0')
    {
        str[i++] = lexer->current_char;
        lexer_advance(lexer);
    }

    if (lexer->current_char == '"')
    {
        lexer_advance(lexer);
    }

    str[i] = '\0';
    return str;
}

char *lexer_collect_number(Lexer *lexer)
{
    char *num = (char *)malloc(256 * sizeof(char));
    int i = 0;

    while (isdigit(lexer->current_char))
    {
        num[i++] = lexer->current_char;
        lexer_advance(lexer);
    }

    num[i] = '\0';
    return num;
}

char *lexer_collect_identifier(Lexer *lexer)
{
    char *identifier = (char *)malloc(256 * sizeof(char));
    int i = 0;

    while (isalnum(lexer->current_char) || lexer->current_char == '_')
    {
        identifier[i++] = lexer->current_char;
        lexer_advance(lexer);
    }

    identifier[i] = '\0';
    return identifier;
}

Token *lexer_get_next_token(Lexer *lexer)
{
    while (lexer->current_char != '\0')
    {
        // Skip whitespace
        if (isspace(lexer->current_char))
        {
            lexer_skip_whitespace(lexer);
            continue;
        }

        int current_line = lexer->line;
        int current_column = lexer->column;

        if (isdigit(lexer->current_char))
        {
            char *num = lexer_collect_number(lexer);
            Token *token = create_token(TOKEN_NUMBER_LITERAL, num, current_line, current_column);
            free(num);
            return token;
        }

        if (isalpha(lexer->current_char) || lexer->current_char == '_')
        {
            char *identifier = lexer_collect_identifier(lexer);
            TokenType type = TOKEN_IDENTIFIER;

            // Check for keywords
            if (strcmp(identifier, "num") == 0)
                type = TOKEN_NUM;
            else if (strcmp(identifier, "str") == 0)
                type = TOKEN_STR;
            else if (strcmp(identifier, "show") == 0)
                type = TOKEN_SHOW;
            else if (strcmp(identifier, "when") == 0)
                type = TOKEN_WHEN;
            else if (strcmp(identifier, "otherwise") == 0)
                type = TOKEN_OTHERWISE;
            else if (strcmp(identifier, "repeat") == 0)
                type = TOKEN_REPEAT;
            else if (strcmp(identifier, "ask") == 0)
                type = TOKEN_ASK;

            Token *token = create_token(type, identifier, current_line, current_column);
            free(identifier);
            return token;
        }

        if (lexer->current_char == '"')
        {
            char *str = lexer_collect_string(lexer);
            Token *token = create_token(TOKEN_STRING_LITERAL, str, current_line, current_column);
            free(str);
            return token;
        }
        char current = lexer->current_char;
        lexer_advance(lexer);

        switch (current)
        {
        case '+':
            return create_token(TOKEN_PLUS, "+", current_line, current_column);
        case '-':
            if (isdigit(lexer->current_char))
            {
                // Collect the negative number
                char *num = (char *)malloc(256 * sizeof(char));
                int i = 0;
                num[i++] = '-'; // Add the minus sign

                while (isdigit(lexer->current_char))
                {
                    num[i++] = lexer->current_char;
                    lexer_advance(lexer);
                }

                num[i] = '\0';
                Token *token = create_token(TOKEN_NUMBER_LITERAL, num, current_line, current_column);
                free(num);
                return token;
            }
            return create_token(TOKEN_MINUS, "-", current_line, current_column);
        case '*':
            return create_token(TOKEN_MULTIPLY, "*", current_line, current_column);
        case '/':
            if (lexer->current_char == '/')
            {
                lexer_advance(lexer);
                while (lexer->current_char != '\0' && lexer->current_char != '\n')
                {
                    lexer_advance(lexer);
                }
                if (lexer->current_char == '\n')
                {
                    lexer_advance(lexer);
                }
                continue;
            }
            return create_token(TOKEN_DIVIDE, "/", current_line, current_column);
        case '<':
            return create_token(TOKEN_LESS, "<", current_line, current_column);
        case '>':
            return create_token(TOKEN_GREATER, ">", current_line, current_column);
        case '=':
            return create_token(TOKEN_EQUALS, "=", current_line, current_column);
        case ';':
            return create_token(TOKEN_SEMICOLON, ";", current_line, current_column);
        case '(':
            return create_token(TOKEN_LPAREN, "(", current_line, current_column);
        case ')':
            return create_token(TOKEN_RPAREN, ")", current_line, current_column);
        case '{':
            return create_token(TOKEN_LBRACE, "{", current_line, current_column);
        case '}':
            return create_token(TOKEN_RBRACE, "}", current_line, current_column);
        }

        printf("Error: Unknown character '%c' at line %d, column %d\n",
               current, current_line, current_column);
    }

    return create_token(TOKEN_EOF, "", lexer->line, lexer->column);
}

void free_lexer(Lexer *lexer)
{
    if (lexer)
    {
        free(lexer->source);
        free(lexer);
    }
}