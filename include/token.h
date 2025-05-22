#ifndef TOKEN_H
#define TOKEN_H
typedef enum
{
    // Data types
    TOKEN_NUM,
    TOKEN_STR,
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LESS,
    TOKEN_GREATER,
    // Keywords
    TOKEN_SHOW,      // print
    TOKEN_WHEN,      // if
    TOKEN_OTHERWISE, // else
    TOKEN_REPEAT,    // for
    TOKEN_ASK,       // input
               // Others
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

#endif