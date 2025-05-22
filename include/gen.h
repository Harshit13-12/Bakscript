#ifndef GEN_H
#define GEN_H

#include "tac.h"

// Register allocation structure
typedef struct
{
    char *name;     // Register name
    char *variable; // Variable currently in this register
    bool is_free;   // Whether the register is available
} Register;

// Code generation context
typedef struct
{
    Register *registers; // Array of available registers
    int reg_count;       // Number of available registers
    int current_reg;     // Current register being used
    char *output;        // Generated code
    int output_size;     // Size of output buffer
    int output_pos;      // Current position in output buffer
} GenContext;

// Function declarations
GenContext *create_gen_context(void);
void free_gen_context(GenContext *context);
char *generate_code(TAC *tac);
void append_code(GenContext *context, const char *format, ...);
Register *allocate_register(GenContext *context, const char *variable);
void free_register(GenContext *context, Register *reg);

#endif // GEN_H