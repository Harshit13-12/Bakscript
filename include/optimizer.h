#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

// Basic optimization functions
TAC *optimize_tac(TAC *tac);

// Specific optimization passes
TAC *constant_folding(TAC *tac);
TAC *dead_code_elimination(TAC *tac);
TAC *common_subexpression_elimination(TAC *tac);
TAC *strength_reduction(TAC *tac);

// Helper functions
int is_constant_value(const char *value);
int is_temp_var(const char *var);
int is_same_expression(TAC *tac1, TAC *tac2);

#endif // OPTIMIZER_H