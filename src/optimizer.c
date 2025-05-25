#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/optimizer.h"

// Helper function to check if a value is a constant
int is_constant_value(const char *value)
{
    if (!value)
        return 0;
    if (value[0] >= '0' && value[0] <= '9')
        return 1;
    if (value[0] == '"')
        return 1;
    return 0;
}

// Helper function to check if a variable is temporary
int is_temp_var(const char *var)
{
    if (!var)
        return 0;
    return var[0] == 't' && var[1] >= '0' && var[1] <= '9';
}

// Helper function to check if two expressions are the same
int is_same_expression(TAC *tac1, TAC *tac2)
{
    if (!tac1 || !tac2)
        return 0;
    if (tac1->op != tac2->op)
        return 0;
    if (strcmp(tac1->arg1, tac2->arg1) != 0)
        return 0;
    if (strcmp(tac1->arg2, tac2->arg2) != 0)
        return 0;
    return 1;
}

// Constant folding optimization
TAC *constant_folding(TAC *tac)
{
    if (!tac)
        return NULL;

    TAC *current = tac;
    while (current)
    {
        if ((current->op == TAC_ADD || current->op == TAC_SUB ||
             current->op == TAC_MUL || current->op == TAC_DIV) &&
            is_constant_value(current->arg1) && is_constant_value(current->arg2))
        {

            int val1 = atoi(current->arg1);
            int val2 = atoi(current->arg2);
            int result;

            switch (current->op)
            {
            case TAC_ADD:
                result = val1 + val2;
                break;
            case TAC_SUB:
                result = val1 - val2;
                break;
            case TAC_MUL:
                result = val1 * val2;
                break;
            case TAC_DIV:
                if (val2 == 0)
                {
                    fprintf(stderr, "Error: Division by zero\n");
                    return tac;
                }
                result = val1 / val2;
                break;
            default:
                break;
            }
            char result_str[32];
            sprintf(result_str, "%d", result);
            current->op = TAC_ASSIGN;
            current->arg1 = strdup(result_str);
            current->arg2 = NULL;
        }
        else if ((current->op == TAC_NEG) && is_constant_value(current->arg1))
        {
            int val = atoi(current->arg1);
            char result_str[32];
            sprintf(result_str, "%d", -val);
            current->op = TAC_ASSIGN;
            current->arg1 = strdup(result_str);
            current->arg2 = NULL;
        }
        else if ((current->op == TAC_LESS || current->op == TAC_GREATER) &&
                 is_constant_value(current->arg1) && is_constant_value(current->arg2))
        {
            int val1 = atoi(current->arg1);
            int val2 = atoi(current->arg2);
            int result;

            switch (current->op)
            {
            case TAC_LESS:
                result = val1 < val2;
                break;
            case TAC_GREATER:
                result = val1 > val2;
                break;
            default:
                break;
            }

            char result_str[32];
            sprintf(result_str, "%d", result);
            current->op = TAC_ASSIGN;
            current->arg1 = strdup(result_str);
            current->arg2 = NULL;
        }
        current = current->next;
    }
    return tac;
}

// Dead code elimination
TAC *dead_code_elimination(TAC *tac)
{
    if (!tac)
        return NULL;

    int *used = (int *)calloc(1000, sizeof(int));
    int *defined = (int *)calloc(1000, sizeof(int)); 
    TAC *current = tac;
    while (current)
    {
        if (current->result && is_temp_var(current->result))
        {
            defined[atoi(current->result + 1)] = 1;
        }

        if (current->arg1 && is_temp_var(current->arg1))
        {
            used[atoi(current->arg1 + 1)] = 1;
        }
        if (current->arg2 && is_temp_var(current->arg2))
        {
            used[atoi(current->arg2 + 1)] = 1;
        }
        if (current->op == TAC_ASSIGN && current->result && !is_temp_var(current->result))
        {
            if (current->arg1 && is_temp_var(current->arg1))
            {
                used[atoi(current->arg1 + 1)] = 1;
            }
        }

        current = current->next;
    }

    TAC *prev = NULL;
    current = tac;
    while (current)
    {
        bool should_remove = false;

        if (current->op == TAC_ASSIGN &&
            current->result &&
            is_temp_var(current->result))
        {

            int var_num = atoi(current->result + 1);
            should_remove = !used[var_num];
        }

        if (should_remove)
        {
            if (prev)
            {
                prev->next = current->next;
                TAC *to_free = current;
                current = current->next;
                tac_free(to_free);
            }
            else
            {
                tac = current->next;
                TAC *to_free = current;
                current = current->next;
                tac_free(to_free);
            }
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }

    free(used);
    free(defined);
    return tac;
}

// Common subexpression elimination
TAC *common_subexpression_elimination(TAC *tac)
{
    if (!tac)
        return NULL;

    TAC *current = tac;
    while (current)
    {
        if (current->op == TAC_ADD || current->op == TAC_SUB ||
            current->op == TAC_MUL || current->op == TAC_DIV)
        {

            TAC *search = current->next;
            while (search)
            {
                if (is_same_expression(current, search))
                {
                    search->op = TAC_ASSIGN;
                    search->arg1 = strdup(current->result);
                    search->arg2 = NULL;
                }
                search = search->next;
            }
        }
        current = current->next;
    }
    return tac;
}

// Strength reduction
TAC *strength_reduction(TAC *tac)
{
    if (!tac)
        return NULL;

    TAC *current = tac;
    while (current)
    {
        if (current->op == TAC_MUL &&
            ((strcmp(current->arg1, "2") == 0) ||
             (strcmp(current->arg2, "2") == 0)))
        {

            current->op = TAC_ADD;
            if (strcmp(current->arg1, "2") == 0)
            {
                current->arg1 = current->arg2;
                current->arg2 = current->arg1;
            }
        }
        current = current->next;
    }
    return tac;
}

// Main optimization function that applies all passes
TAC *optimize_tac(TAC *tac)
{
    if (!tac)
        return NULL;
    tac = constant_folding(tac);
    tac = common_subexpression_elimination(tac);
    tac = strength_reduction(tac);
    tac = dead_code_elimination(tac);

    return tac;
}