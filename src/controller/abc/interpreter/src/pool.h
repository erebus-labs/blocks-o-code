/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/


/// \file pool.h
/// \brief Allocate memory for building AST
///
/// Creates a stack-allocated pool for building an
/// an abstract syntax tree by providing pointers
/// to the next available struct on the stack on
/// demand. The AST is balanced so that each struct
/// can be of the same size, simplifying stack
/// allocation.
///
/// All nodes are assumed to be binary operators.
/// Terminal nodes can be numbered literals or
/// variables, meaning each of their operands are
/// a special type called "empty". Unary operators
/// have one of their operands typed empty as well.

#ifndef ABC_POOL_H
#define ABC_POOL_H

#define MEM_POOL_SIZE 2048

///
/// Defines the semantic type of a node
typedef enum {
    ASTEmpty = 0,       ///< Terminal pointer
    ASTNum,             ///< A number
    ASTVar,             ///< A variable
    ASTExp,             ///< A high-level expression
    ASTList,            ///< A list of expressions
    ASTConditional,     ///< An if...else statement
    ASTLoop             ///< A loop
} ASTNode_t;

///
/// A node on the AST
typedef struct _ASTNode {
    ASTNode_t t;            ///< The datatype for the node data
    union {
        double  num;        ///< The number if a terminal literal

        char    op;         ///< The char stored from the command.
                            ///< Note that Variables are also stored
                            ///< here and just return their opcode

        struct _ASTNode* e; ///< The expression for the current statement

    } data;                 ///< The data associated with the node

    int linenum;            ///< The linenumber of the statement

    struct _ASTNode* lop;   ///< The left operand/previous list item
    struct _ASTNode* rop;   ///< The right operand/next list item
} ASTNode;

///
/// Get a new node
ASTNode* ast_node_new(void);

// The following are just constructors for `ast_node_new()` so they're
// undocumented (because they're redundant).
ASTNode* ast_node_new_op(char c, ASTNode* lop, ASTNode* rop);
ASTNode* ast_node_new_var(char c, ASTNode* lop, ASTNode* rop);
ASTNode* ast_node_new_literal(double n, ASTNode* lop, ASTNode* rop);
ASTNode* ast_node_new_list(ASTNode* e, int linenum);
ASTNode* ast_node_list_append(ASTNode* e, ASTNode* rop);
ASTNode* ast_node_empty();
ASTNode* ast_node_conditional(ASTNode* e, ASTNode* lop, ASTNode* rop);
ASTNode* ast_node_new_loop(char c, ASTNode* lop, ASTNode* rop);

#endif
