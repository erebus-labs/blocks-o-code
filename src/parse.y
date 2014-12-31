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
%type <SEMNode> exp explist

/* Precedence *****************/

%left BLOCK                     /* A list of expressions    */
%left COND_ELSE                 /* A conditional w/ else    */
%left COND                      /* A conditional w/o else   */
%left STATLIST
%left STAT                      /* A statement              */
%left SUBEXP                    /* Expression in parens     */
%precedence EVAR                /* Empty variable statement */
%left '@' 'd'                   /* While loops              */
%left '$'                       /* Repeat loop              */
%left '?'                       /* Conditional Symbol       */
%left ':'                       /* Assignment Operator      */
%left 'p' 'c'                   /* Print Action             */
%left '=' '>' '<' 'l' 'g' '~'   /* Comparision Operators    */
%left '-' '+'                   /* Arithmetic Operators     */
%left '*' '/'                   /*                          */
%right '^' '%'                  /* Arithmetic Operators     */
%precedence NNEG                /* Unary Numerical Negation */
%right '!'                      /* Unary Logical Negation   */

/* Semantics ******************/
%%

input:
    blank explist {
        g_ast_root = $2;
    }
    | explist {
        g_ast_root = $1;
    }
;

blank:
      '\n'
    | '\n' blank
;

explist:
      explist ';' %prec STATLIST {
        $$ = ast_node_new_list($1);
    }
    |  explist '\n' %prec STATLIST {
        $$ = ast_node_new_list($1);
    }
    | explist exp %prec STAT {
        $$ = ast_node_list_append($1, ast_node_new_list($2));
    }
    | exp %prec STAT {
        $$ = ast_node_new_list($1);
    }
;

exp:
    NUM {
        $$ = ast_node_new_literal($1, 0, 0);
    }
    | VAR %prec EVAR {
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
    | exp '@' exp {
        $$ = ast_node_new_loop('@', $1, $3);
    }
    | exp 'd' exp {
        $$ = ast_node_new_loop('d', $1, $3);
    }
    | exp '$' exp {
        $$ = ast_node_new_loop('$', $1, $3);
    }
    | '(' exp ')' %prec SUBEXP {
        $$ = $2;
    }
    | 'p' exp {
        $$ = ast_node_new_op('p', 0, $2);
    }
    | 'c' exp {
        $$ = ast_node_new_op('c', 0, $2);
    }
    | VAR ':' exp {
        $$ = ast_node_new_op(':', ast_node_new_var($1,0,0), $3);
    }
    | '{' explist '}' %prec BLOCK {
        $$ = ast_node_new_list($2);
    }
    | '{' blank explist '}' %prec BLOCK {
        $$ = ast_node_new_list($3);
    }
    | exp '?' exp %prec COND {
        $$ = ast_node_conditional($1, $3, 0);
    }
    | exp '?' exp ':' exp %prec COND_ELSE {
        $$ = ast_node_conditional($1, $3, $5);
    }
;

%%

