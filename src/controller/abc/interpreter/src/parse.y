/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Gregs's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/

/* Declarations ***************/
%{
    #include <stdio.h>
    #include <math.h>

    #include "parse.lex.h"
    #include "globals.h"

    void yyerror (char const*);

    #include "eval.h"
    #include "pool.h"

    extern struct _ASTNode* g_ast_root;

    int errorline;
    int errorcolumn;
    char* errortext;
    int unrecognized = 0;

    extern char expects_condition;
%}

/* Types **********************/

%define parse.lac full
//%define parse.error verbose
%locations

%union {
    double SEMDouble;
    char SEMChar;
    struct _ASTNode* SEMNode;
};

/* Value for literals is a double */
%token <SEMDouble> NUM

/* One of 6 variables [A-F]  */
%token <SEMChar> VAR
%token <SEMChar> EX_PLUS EX_MINUS EX_MULT EX_DIV EX_POW EX_MOD
%token <SEMChar> AS_PLUS AS_MINUS AS_MULT AS_DIV AS_POW AS_MOD
%token <SEMChar> EX_EQ   EX_NE    EX_LT   EX_GT  EX_LE  EX_GE
%token <SEMChar> WHILE END_WHILE IF ELSE END_IF SAY OUTPUT
%token <SEMChar> RPAREN LPAREN EX_CLAIMATION
%token <SEMChar> ERROR INTERACTIVE_QUIT

/* Value for expressions */
%type <SEMNode> EXPRESSION STATEMENT ENDWHILE_STATEMENT_LIST ENDIF_STATEMENT_LIST ELSE_STATEMENT_LIST STATEMENT_LIST



/* Precedence *****************/
%left GLOBAL
%left GLOBAL_STAT
%left GLOBAL_STATLIST
%left STAT                      /* A statement              */
%left STATLIST
%left WHILE IF
%left IF_ONLY
%left IF_ELSE
%left END_IF END_WHILE
%left OUTPUT SAY AS_PLUS AS_MINUS AS_MULT AS_DIV AS_POW AS_MOD
%left SUBEXP                    /* Expression in parens     */
%left EVAR                /* Empty variable statement */
%left EX_EQ EX_NE EX_LT EX_GT EX_LE EX_GE /* Comparision Operators    */
%left EX_PLUS EX_MINUS          /* Arithmetic Operators     */
%left EX_MULT EX_DIV            /*                          */
%right EX_POW EX_MOD            /* Arithmetic Operators     */
%left NNOT                /* Unary Numerical Negation */
%left NNEG                /* Unary Numerical Negation */

/* Semantics ******************/
%%

input:
      STATEMENT_LIST {
        DEBUG("    (Parser Responds: Found 'Global' statement list)\n");
        DEBUG("Parser Exclaims 'Parse Complete!'\n");
        g_ast_root = $1;
    }
;

STATEMENT_LIST:
      STATEMENT_LIST STATEMENT {
        $$ = ast_node_list_append($1, ast_node_new_list($2, yylineno - 1));
    }
    | STATEMENT {
        $$ = ast_node_new_list($1, yylineno - 1);
    }
;

ENDIF_STATEMENT_LIST:
      STATEMENT_LIST END_IF {
        DEBUG("    (Parser Responds: Found 'if' statement list)\n");
        $$ = $1;
    }
;

ELSE_STATEMENT_LIST:
      STATEMENT_LIST ELSE  {
        DEBUG("    (Parser Responds: Found 'else' statement list)\n");
        $$ = $1;
    }
;

ENDWHILE_STATEMENT_LIST:
      STATEMENT_LIST END_WHILE {
        DEBUG("    (Parser Responds: Found 'while' statement list)\n");
        $$ = $1;
    }
;

STATEMENT:
      WHILE   EXPRESSION ENDWHILE_STATEMENT_LIST {
        DEBUG("    (Parser Responds: Found 'while' statement)\n");
        $$ = ast_node_new_loop('@', $2, $3);
    }
    | IF      EXPRESSION ENDIF_STATEMENT_LIST {
        DEBUG("    (Parser Responds: Found 'if' statement)\n");
        $$ = ast_node_conditional($2, $3, 0);
    }
    | IF      EXPRESSION ELSE_STATEMENT_LIST ENDIF_STATEMENT_LIST {
        DEBUG("    (Parser Responds: Found 'if...Else' statement)\n");
        $$ = ast_node_conditional($2, $3, $4);
    }
    | VAR     EXPRESSION {
        DEBUG("    (Parser Responds: Found 'assignment' statement)\n");
        $$ = ast_node_new_op(':', ast_node_new_var($1,0,0), $2);
    }
    | SAY     EXPRESSION {
        DEBUG("    (Parser Responds: Found 'say' statement)\n");
        $$ = ast_node_new_op('p', 0, $2);
    }
    | OUTPUT  EXPRESSION EXPRESSION {
        DEBUG("    (Parser Responds: Found 'output' statement)\n");
        $$ = ast_node_new_op('o', $2, $3);
    }
    | VAR     AS_PLUS    EXPRESSION {
        DEBUG("    (Parser Responds: Found '+=' statement)\n");
        $$ = ast_node_new_op('A', ast_node_new_var($1,0,0), $3);
    }
    | VAR     AS_MINUS   EXPRESSION {
        DEBUG("    (Parser Responds: Found '-=' statement)\n");
        $$ = ast_node_new_op('B', ast_node_new_var($1,0,0), $3);
    }
    | VAR     AS_MULT    EXPRESSION {
        DEBUG("    (Parser Responds: Found '*=' statement)\n");
        $$ = ast_node_new_op('C', ast_node_new_var($1,0,0), $3);
    }
    | VAR     AS_DIV     EXPRESSION {
        DEBUG("    (Parser Responds: Found '/=' statement)\n");
        $$ = ast_node_new_op('D', ast_node_new_var($1,0,0), $3);
    }
    | VAR     AS_POW     EXPRESSION {
        DEBUG("    (Parser Responds: Found '^=' statement)\n");
        $$ = ast_node_new_op('E', ast_node_new_var($1,0,0), $3);
    }
    | VAR     AS_MOD     EXPRESSION {
        DEBUG("    (Parser Responds: Found '%=' statement)\n");
        $$ = ast_node_new_op('F', ast_node_new_var($1,0,0), $3);
    }
;

EXPRESSION:
    NUM {
        expects_condition = 0;
        DEBUG("    (Parser Responds: Found 'number %g' expresson)\n", $1);
        $$ = ast_node_new_literal($1, 0, 0);
    }
    | VAR {
        expects_condition = 0;
        DEBUG("    (Parser Responds: Found 'variable %d' expresson)\n", $1);
        $$ = ast_node_new_var($1, 0, 0);
    }
    | EXPRESSION EX_PLUS EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('+', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix +' expresson)\n");
    }
    | EXPRESSION EX_MINUS EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('-', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix -' expresson)\n");
    }
    | EXPRESSION EX_MULT EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('*', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix *' expresson)\n");
    }
    | EXPRESSION EX_DIV EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('/', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix /' expresson)\n");
    }
    | EX_MINUS EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('-', 0, $2);
        DEBUG("    (Parser Responds: Found 'unary -' expresson)\n");
    }
    | EXPRESSION EX_POW EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('^', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix ^' expresson)\n");
    }
    | EXPRESSION EX_MOD EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('%', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix %' expresson)\n");
    }
    | EXPRESSION EX_EQ EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('=', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix ==' expresson)\n");
    }
    | EXPRESSION EX_GT EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('>', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix <' expresson)\n");
    }
    | EXPRESSION EX_LT EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('<', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix >' expresson)\n");
    }
    | EXPRESSION EX_LE EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('l', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix <=' expresson)\n");
    }
    | EXPRESSION EX_GE EXPRESSION {
        expects_condition = 0;
        DEBUG("    (Parser Responds: Found 'infix >=' expresson)\n");
        $$ = ast_node_new_op('g', $1, $3);
    }
    | EXPRESSION EX_NE EXPRESSION {
        expects_condition = 0;
        DEBUG("    (Parser Responds: Found 'infix !=' expresson)\n");
        $$ = ast_node_new_op('~', $1, $3);
    }
    | EX_CLAIMATION EXPRESSION {
        expects_condition = 0;
        $$ = ast_node_new_op('!', 0, $2);
        DEBUG("    (Parser Responds: Found 'unary !' expresson)\n");
    }
    | LPAREN EXPRESSION RPAREN {
        expects_condition = 0;
        DEBUG("    [Parser Responds: Found '(subexpresson)' expresson]\n");
        $$ = $2;
    }
    | ERROR {
        YYABORT;
    }
    | INTERACTIVE_QUIT {
        YYABORT;
    }
;

%%

