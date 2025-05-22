#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

TAC *optimize_tac(TAC *tac);
TAC *constant_folding(TAC *tac);
TAC *dead_code_elimination(TAC *tac);
TAC *common_subexpression_elimination(TAC *tac);
TAC *strength_reduction(TAC *tac);
int is_constant_value(const char *value);
int is_temp_var(const char *var);
int is_same_expression(TAC *tac1, TAC *tac2);

#endif 