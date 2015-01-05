/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/


#include <stdlib.h>
#include <stdio.h>

#include "pool.h"

// Stack-Allocated memory pool
ASTNode g_memory_pool[MEM_POOL_SIZE] = {};

// Current free block
ASTNode* g_memory_pool_index = &g_memory_pool[0];

ASTNode* ast_node_new(void) {
    // Make sure we haven' run out of stuff
    if (g_memory_pool_index == &g_memory_pool[MEM_POOL_SIZE]) {
        fprintf(stderr, "ERROR: ran out of memory :(\n");
        exit(1);
    }
    return g_memory_pool_index++;
}

ASTNode* ast_node_new_op(char c, ASTNode* lop, ASTNode* rop) {
    ASTNode* node = ast_node_new();
    node->t = ASTExp;
    node->data.op = c;
    node->lop = lop;
    node->rop = rop;
    return node;
}

ASTNode* ast_node_new_var(char c, ASTNode* lop, ASTNode* rop) {
    ASTNode* node = ast_node_new();
    node->t = ASTVar;
    node->data.op = c;
    node->lop = 0;
    node->rop = rop;
    return node;
}

ASTNode* ast_node_new_literal(double n, ASTNode* lop, ASTNode* rop) {
    ASTNode* node = ast_node_new();
    node->t = ASTNum;
    node->data.num = n;
    node->lop = 0;
    node->rop = 0;
    return node;
}

ASTNode* ast_node_new_list(ASTNode* e) {
    ASTNode* node = ast_node_new();
    node->t = ASTList;
    node->data.e = e;
    node->lop = 0;
    node->rop = 0;
    return node;
}

ASTNode* ast_node_list_append(ASTNode* e, ASTNode* rop) {
    ASTNode* node = ast_node_new();
    node->t = ASTList;
    node->data.e = e;
    node->lop = 0;
    node->rop = rop;
    return node;
}

ASTNode* ast_node_empty() {
    ASTNode* node = ast_node_new();
    node->t = ASTEmpty;
    node->lop = 0;
    node->rop = 0;
    return node;
   
}

ASTNode* ast_node_conditional(ASTNode* e, ASTNode* lop, ASTNode* rop) {
    ASTNode* node = ast_node_new();
    node->t = ASTConditional;
    node->data.e = e;
    node->lop = lop;
    node->rop = rop;
    return node;
}

ASTNode* ast_node_new_loop(char c, ASTNode* lop, ASTNode* rop) {
    ASTNode* node = ast_node_new_op(c, lop, rop);
    node->t = ASTLoop;
    return node;
}


