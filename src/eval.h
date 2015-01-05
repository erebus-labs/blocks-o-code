/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/

/// \file eval.h
/// \brief Evaulate ABC AST
///
/// Defines the global `g_ast_root` which is the
/// trunk of the AST. Just call eval_expr on it
/// to execute the AST.


#ifndef ABC_EVAL_H
#define ABC_EVAL_H

#include "pool.h"

///
/// Evaulates an AST
/// @param expr The trunk of code to evaluate
double eval_expr(ASTNode* expr);

///
/// The trunk of the ast
extern ASTNode* g_ast_root;

#endif
