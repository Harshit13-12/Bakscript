#ifndef AST_H
#define AST_H

// Node types for our AST
typedef enum
{
    // Expressions
    NODE_NUMBER,
    NODE_STRING,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_FUNCTION_CALL,

    // Statements
    NODE_VARIABLE_DECLARATION,
    NODE_IF_STATEMENT,
    NODE_FOR_LOOP,
    NODE_BLOCK,
    NODE_PROGRAM
} NodeType;

// Forward declaration of Node structure
typedef struct Node Node;

// Binary operator types
typedef enum
{
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_LESS,
    OP_GREATER,
    OP_ASSIGN
} BinaryOpType;

// Common node information
typedef struct
{
    int line;
    int column;
} NodeInfo;

// Structure for binary operations (e.g., +, -, *, /, <, >)
typedef struct
{
    BinaryOpType op;
    Node *left;
    Node *right;
    NodeInfo info;
} BinaryOpNode;

// Structure for number literals
typedef struct
{
    int value;
    NodeInfo info;
} NumberNode;

// Structure for string literals
typedef struct
{
    char *value;
    NodeInfo info;
} StringNode;

// Structure for identifiers (variable names)
typedef struct
{
    char *name;
    NodeInfo info;
} IdentifierNode;

// Structure for function calls (show, ask)
typedef struct
{
    char *name;
    Node **arguments;
    int arg_count;
    NodeInfo info;
} FunctionCallNode;

// Structure for variable declarations
typedef struct
{
    char *type;
    char *name;
    Node *initializer;
    NodeInfo info;
} VariableDeclarationNode;

// Structure for if statements
typedef struct
{
    Node *condition;
    Node *if_body;
    Node *else_body;
    NodeInfo info;
} IfStatementNode;

// Structure for for loops
typedef struct
{
    Node *initializer;
    Node *condition;
    Node *increment;
    Node *body;
    NodeInfo info;
} ForLoopNode;

// Structure for blocks of statements
typedef struct
{
    Node **statements;
    int count;
    NodeInfo info;
} BlockNode;

// Structure for the entire program
typedef struct
{
    Node **statements;
    int count;
    NodeInfo info;
} ProgramNode;

// Main Node structure
struct Node
{
    NodeType type;
    union
    {
        NumberNode number;
        StringNode string;
        IdentifierNode identifier;
        BinaryOpNode binary_op;
        FunctionCallNode function_call;
        VariableDeclarationNode var_decl;
        IfStatementNode if_stmt;
        ForLoopNode for_loop;
        BlockNode block;
        ProgramNode program;
    };
};

// Function declarations for creating AST nodes
Node *create_number_node(int value, int line, int column);
Node *create_string_node(const char *value, int line, int column);
Node *create_identifier_node(const char *name, int line, int column);
Node *create_binary_op_node(BinaryOpType op, Node *left, Node *right, int line, int column);
Node *create_function_call_node(const char *name, Node **arguments, int arg_count, int line, int column);
Node *create_var_decl_node(const char *type, const char *name, Node *initializer, int line, int column);
Node *create_if_node(Node *condition, Node *if_body, Node *else_body, int line, int column);
Node *create_for_node(Node *initializer, Node *condition, Node *increment, Node *body, int line, int column);
Node *create_block_node(Node **statements, int count, int line, int column);
Node *create_program_node(Node **statements, int count);

// Function to free AST nodes
void free_node(Node *node);

#endif // AST_H
