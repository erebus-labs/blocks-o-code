/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Tyler's proposal for syntax/symantics

(C)2014 Erubus Labs
* For internal use only (subject to change)

****************************************/

/* Declarations ***************/
%{
    #include <stdio.h>
    #include <math.h>

    int yylex (void);
    void yyerror (char const *);

    #include "eval.h"
    #include "pool.h"

    extern struct _ASTNode* g_ast_root;
%}

/* Types **********************/

%union {
    double SEMDouble;
    char SEMChar;
    struct _ASTNode* SEMNode;
};

/* Value for literals is a double */
%token <SEMDouble> NUM

/* One of 6 variables [A-F]  */
%token <SEMChar> VAR

/* Value for expressions */
%type <SEMNode> exp statement

/* Precedence *****************/
%left ':'                       /* Assignment Operator      */
%left 'p'                       /* Print Action             */
%left '=' '>' '<' 'l' 'g' '~'   /* Comparision Operators    */
%left '-' '+'                   /* Arithmetic Operators     */
%left '*' '/'                   /*                          */
%precedence NNEG                /* Unary Numerical Negation */
%right '^' '%'                  /* Arithmetic Operators     */
%right '!'                      /* Unary Logical Negation   */
%precedence EVAR                /* Empty variable statement */


/* Semantics ******************/
%%

input:
    statement {
        g_ast_root = $1;
    }
;

statement:
      statement '\n' {
        $$ = ast_node_new_list($1);
    }
    | statement statement {
        $$ = ast_node_list_append($1, $2);
    }
    | exp {
        $$ = ast_node_new_list($1);
    }
;

exp:
    NUM {
        $$ = ast_node_new_literal($1, 0, 0);
    }
    | VAR {
        $$ = ast_node_new_var($1, 0, 0);
    }
    | exp '+' exp {
        $$ = ast_node_new_op('+', $1, $3);
    }
    | exp '-' exp {
        $$ = ast_node_new_op('-', $1, $3);
    }
    | exp '*' exp {
        $$ = ast_node_new_op('*', $1, $3);
    }
    | exp '/' exp {
        $$ = ast_node_new_op('/', $1, $3);
    }
    | '-' exp %prec NNEG {
        $$ = ast_node_new_op('-', 0, $2);
    }
    | '!' exp {
        $$ = ast_node_new_op('!', 0, $2);
    }
    | exp '^' exp {
        $$ = ast_node_new_op('^', $1, $3);
    }
    | exp '%' exp {
        $$ = ast_node_new_op('%', $1, $3);
    }
    | exp '=' exp {
        $$ = ast_node_new_op('=', $1, $3);
    }
    | exp '>' exp {
        $$ = ast_node_new_op('>', $1, $3);
    }
    | exp '<' exp {
        $$ = ast_node_new_op('<', $1, $3);
    }
    | exp 'l' exp {
        $$ = ast_node_new_op('l', $1, $3);
    }
    | exp 'g' exp {
        $$ = ast_node_new_op('g', $1, $3);
    }
    | exp '~' exp {
        $$ = ast_node_new_op('~', $1, $3);
    }
    | '(' exp ')' {
        $$ = $2;
    }
    | 'p' exp {
        $$ = ast_node_new_op('p', 0, $2);
    }
    | VAR ':' exp {
        $$ = ast_node_new_op(':', ast_node_new_var($1,0,0), $3);
    }

;

%%

