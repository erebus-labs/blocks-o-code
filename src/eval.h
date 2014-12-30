/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Tyler's proposal for syntax/symantics

(C)2014 Erubus Labs
* For internal use only (subject to change)

****************************************/


#ifndef ABC_EVAL_H
#define ABC_EVAL_H

#include "pool.h"

double eval_expr(ASTNode* expr);
extern ASTNode* g_ast_root;

#endif
