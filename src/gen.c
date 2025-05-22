#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "../include/gen.h"

#define INITIAL_OUTPUT_SIZE 1024
#define MAX_REGISTERS 8

GenContext *create_gen_context(void)
{
    GenContext *context = (GenContext *)malloc(sizeof(GenContext));
    if (!context)
    {
        fprintf(stderr, "Error: Memory allocation failed for GenContext\n");
        exit(1);
    }

    context->reg_count = MAX_REGISTERS;
    context->registers = (Register *)malloc(sizeof(Register) * MAX_REGISTERS);
    if (!context->registers)
    {
        fprintf(stderr, "Error: Memory allocation failed for registers\n");
        free(context);
        exit(1);
    }

    const char *reg_names[] = {"rax", "rbx", "rcx", "rdx", "rsi", "r8", "r9"};
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        context->registers[i].name = strdup(reg_names[i]);
        context->registers[i].variable = NULL;
        context->registers[i].is_free = 1; // true
    }

    context->output_size = INITIAL_OUTPUT_SIZE;
    context->output = (char *)malloc(context->output_size);
    if (!context->output)
    {
        fprintf(stderr, "Error: Memory allocation failed for output buffer\n");
        for (int i = 0; i < MAX_REGISTERS; i++)
            free(context->registers[i].name);
        free(context->registers);
        free(context);
        exit(1);
    }
    context->output[0] = '\0';
    context->output_pos = 0;
    context->current_reg = 0;

    return context;
}

void free_gen_context(GenContext *context)
{
    if (!context)
        return;

    for (int i = 0; i < context->reg_count; i++)
    {
        free(context->registers[i].name);
        if (context->registers[i].variable)
            free(context->registers[i].variable);
    }
    free(context->registers);
    free(context->output);
    free(context);
}

void append_code(GenContext *context, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int required_size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (context->output_pos + required_size + 1 > context->output_size)
    {
        size_t new_size = context->output_size * 2;
        while (new_size < context->output_pos + required_size + 1)
            new_size *= 2;

        char *new_output = realloc(context->output, new_size);
        if (!new_output)
        {
            fprintf(stderr, "Error: Memory reallocation failed for output buffer\n");
            exit(1);
        }
        context->output = new_output;
        context->output_size = new_size;
    }

    int written = vsnprintf(context->output + context->output_pos,
                            context->output_size - context->output_pos,
                            format, args);
    if (written > 0)
        context->output_pos += written;

    va_end(args);
}

Register *allocate_register(GenContext *context, const char *variable)
{
    for (int i = 0; i < context->reg_count; i++)
    {
        if (context->registers[i].is_free)
        {
            context->registers[i].is_free = 0;
            if (context->registers[i].variable)
                free(context->registers[i].variable);
            context->registers[i].variable = strdup(variable);
            return &context->registers[i];
        }
    }

    // No free register, do round robin replacement
    Register *reg = &context->registers[context->current_reg];
    context->current_reg = (context->current_reg + 1) % context->reg_count;
    if (reg->variable)
        free(reg->variable);
    reg->variable = strdup(variable);
    return reg;
}

void free_register(GenContext *context, Register *reg)
{
    if (!reg)
        return;
    reg->is_free = 1;
    if (reg->variable)
    {
        free(reg->variable);
        reg->variable = NULL;
    }
}

char *generate_code(TAC *tac)
{
    GenContext *context = create_gen_context();

    // First pass: collect all variable names (excluding string literals)
    char declared[256][64] = {{0}};
    int declared_count = 0;

    TAC *current = tac;
    while (current)
    {
#define ADD_VAR_IF_NOT_DECLARED(var)                                                              \
    do                                                                                            \
    {                                                                                             \
        if ((var) && strlen(var) > 0 && (var)[0] != '"' && !isdigit((var)[0]) && (var)[0] != '-') \
        {                                                                                         \
            int found = 0;                                                                        \
            for (int i = 0; i < declared_count; i++)                                              \
            {                                                                                     \
                if (strcmp(declared[i], (var)) == 0)                                              \
                {                                                                                 \
                    found = 1;                                                                    \
                    break;                                                                        \
                }                                                                                 \
            }                                                                                     \
            if (!found)                                                                           \
                strcpy(declared[declared_count++], (var));                                        \
        }                                                                                         \
    } while (0)

        ADD_VAR_IF_NOT_DECLARED(current->result);
        ADD_VAR_IF_NOT_DECLARED(current->arg1);
        ADD_VAR_IF_NOT_DECLARED(current->arg2);

#undef ADD_VAR_IF_NOT_DECLARED

        current = current->next;
    }

    // Declare variables in .data section
    append_code(context, "section .data\n");
    for (int i = 0; i < declared_count; i++)
    {
        if (strcmp(declared[i], "show") == 0)
            continue;
        if (strncmp(declared[i], "t1", 2) == 0 && strlen(declared[i]) == 2)
            continue;
        append_code(context, "    %s: dq 0\n", declared[i]);
    }

    // Second pass: handle string constants (e.g., t1)
    current = tac;
    while (current)
    {
        if (current->op == TAC_ASSIGN && current->arg1 && current->arg1[0] == '"')
        {
            // Only add once, assuming t1 is only string constant label
            append_code(context, "    t1: db %s, 0\n", current->arg1);
        }
        current = current->next;
    }

    // Text section and external declarations
    append_code(context, "\nsection .text\n");
    append_code(context, "global _start\n");
    append_code(context, "extern show_num\n");
    append_code(context, "extern show_str\n");
    append_code(context, "extern process_exit\n\n");
    append_code(context, "_start:\n");

    // Generate code for each TAC instruction
    current = tac;
    while (current)
    {
        switch (current->op)
        {
        case TAC_ASSIGN:
            if (current->arg1[0] == '"') // string literal
            {
                append_code(context, "    lea rax, [rel t1]\n");
                append_code(context, "    mov [%s], rax\n", current->result);
            }
            else if (isdigit(current->arg1[0]) || current->arg1[0] == '-') // immediate number
            {
                append_code(context, "    mov rax, %s\n", current->arg1);
                append_code(context, "    mov [%s], rax\n", current->result);
            }
            else // variable assignment
            {
                append_code(context, "    mov rax, [%s]\n", current->arg1);
                append_code(context, "    mov [%s], rax\n", current->result);
            }
            break;

        case TAC_CALL:
            if (strcmp(current->arg1, "show") == 0)
            {
                // Check if argument (arg2) is number or string literal
                char *arg = current->arg2;
                if (arg[0] == '"') // string literal
                {
                    // Load address of the string into rcx, call show_str
                    append_code(context, "    lea rcx, [rel %s]\n", arg);
                    append_code(context, "    call show_str\n");
                }
                else if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1])))
                {
                    // Immediate number, move into rcx and call show_num
                    append_code(context, "    mov rcx, %s\n", arg);
                    append_code(context, "    call show_num\n");
                }
                else
                {
                    // Otherwise treat as variable name: load variable value into rcx and call show_num
                    append_code(context, "    mov rcx, [%s]\n", arg);
                    append_code(context, "    call show_num\n");
                }
            }
            break;

        case TAC_ADD:
            append_code(context, "    mov rax, [%s]\n", current->arg1);
            append_code(context, "    add rax, [%s]\n", current->arg2);
            append_code(context, "    mov [%s], rax\n", current->result);
            break;

        case TAC_SUB:
            append_code(context, "    mov rax, [%s]\n", current->arg1);
            append_code(context, "    sub rax, [%s]\n", current->arg2);
            append_code(context, "    mov [%s], rax\n", current->result);
            break;

        case TAC_MUL:
            append_code(context, "    mov rax, [%s]\n", current->arg1);
            append_code(context, "    imul rax, [%s]\n", current->arg2);
            append_code(context, "    mov [%s], rax\n", current->result);
            break;

        case TAC_DIV:
            append_code(context, "    mov rax, [%s]\n", current->arg1);
            append_code(context, "    cqo\n");
            append_code(context, "    idiv qword [%s]\n", current->arg2);
            append_code(context, "    mov [%s], rax\n", current->result);
            break;

        case TAC_IF:
            append_code(context, "    mov rax, [%s]\n", current->arg1);
            append_code(context, "    cmp rax, 0\n");
            append_code(context, "    jne %s\n", current->result);
            break;

        case TAC_GOTO:
            append_code(context, "    jmp %s\n", current->result);
            break;

        case TAC_LABEL:
            append_code(context, "%s:\n", current->result);
            break;

        default:
            // Unknown or unhandled TAC op
            break;
        }
        current = current->next;
    }

    // Exit process call at the end
    append_code(context, "\n    mov rcx, 0\n");
    append_code(context, "    call process_exit\n");

    char *result = strdup(context->output);
    free_gen_context(context);
    return result;
}
