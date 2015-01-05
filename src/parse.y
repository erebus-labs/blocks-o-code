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

    int yylex (void);
    void yyerror (char const *);

    #include "eval.h"
    #include "pool.h"

    extern struct _ASTNode* g_ast_root;

    int errorline;
    int errorcolumn;
    char* errortext;
    int unrecognized = 0;
%}

/* Types **********************/

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
%token <SEMChar> ERROR

/* Value for expressions */
%type <SEMNode> EXPRESSION STATEMENT ENDWHILE_STATEMENT_LIST ENDIF_STATEMENT_LIST ELSE_STATEMENT_LIST STATEMENT_LIST



/* Precedence *****************/
%precedence GLOBAL
%precedence GLOBAL_STAT
%precedence GLOBAL_STATLIST
%left STAT                      /* A statement              */
%left STATLIST
%left WHILE IF
%precedence IF_ONLY
%precedence IF_ELSE
%left OUTPUT END_IF END_WHILE
%left SAY AS_PLUS AS_MINUS AS_MULT AS_DIV AS_POW AS_MOD
%left SUBEXP                    /* Expression in parens     */
%precedence EVAR                /* Empty variable statement */
%left EX_EQ EX_NE EX_LT EX_GT EX_LE EX_GE /* Comparision Operators    */
%left EX_PLUS EX_MINUS          /* Arithmetic Operators     */
%left EX_MULT EX_DIV            /*                          */
%right EX_POW EX_MOD            /* Arithmetic Operators     */
%precedence NNOT                /* Unary Numerical Negation */
%precedence NNEG                /* Unary Numerical Negation */

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
      STATEMENT_LIST STATEMENT %prec STATLIST {
        $$ = ast_node_list_append($1, ast_node_new_list($2));
    }
    | STATEMENT %prec STAT {
        $$ = ast_node_new_list($1);
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
    | IF      EXPRESSION ENDIF_STATEMENT_LIST %prec IF_ONLY {
        DEBUG("    (Parser Responds: Found 'if' statement)\n");
        $$ = ast_node_conditional($2, $3, 0);
    }
    | IF      EXPRESSION ELSE_STATEMENT_LIST ENDIF_STATEMENT_LIST %prec IF_ELSE {
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
        $$ = ast_node_new_literal($1, 0, 0);
        DEBUG("    (Parser Responds: Found 'number %g' expresson)\n", $1);
    }
    | VAR %prec EVAR {
        DEBUG("    (Parser Responds: Found 'variable %d' expresson)\n", $1);
        $$ = ast_node_new_var($1, 0, 0);
    }
    | EXPRESSION EX_PLUS EXPRESSION {
        $$ = ast_node_new_op('+', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix +' expresson)\n");
    }
    | EXPRESSION EX_MINUS EXPRESSION {
        $$ = ast_node_new_op('-', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix -' expresson)\n");
    }
    | EXPRESSION EX_MULT EXPRESSION {
        $$ = ast_node_new_op('*', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix *' expresson)\n");
    }
    | EXPRESSION EX_DIV EXPRESSION {
        $$ = ast_node_new_op('/', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix /' expresson)\n");
    }
    | EX_MINUS EXPRESSION %prec NNEG {
        $$ = ast_node_new_op('-', 0, $2);
        DEBUG("    (Parser Responds: Found 'unary -' expresson)\n");
    }
    | EXPRESSION EX_POW EXPRESSION {
        $$ = ast_node_new_op('^', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix ^' expresson)\n");
    }
    | EXPRESSION EX_MOD EXPRESSION {
        $$ = ast_node_new_op('%', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix %' expresson)\n");
    }
    | EXPRESSION EX_EQ EXPRESSION {
        $$ = ast_node_new_op('=', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix ==' expresson)\n");
    }
    | EXPRESSION EX_GT EXPRESSION {
        $$ = ast_node_new_op('>', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix <' expresson)\n");
    }
    | EXPRESSION EX_LT EXPRESSION {
        $$ = ast_node_new_op('<', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix >' expresson)\n");
    }
    | EXPRESSION EX_LE EXPRESSION {
        $$ = ast_node_new_op('l', $1, $3);
        DEBUG("    (Parser Responds: Found 'infix <=' expresson)\n");
    }
    | EXPRESSION EX_GE EXPRESSION {
        DEBUG("    (Parser Responds: Found 'infix >=' expresson)\n");
        $$ = ast_node_new_op('g', $1, $3);
    }
    | EXPRESSION EX_NE EXPRESSION {
        DEBUG("    (Parser Responds: Found 'infix !=' expresson)\n");
        $$ = ast_node_new_op('~', $1, $3);
    }
    | EX_CLAIMATION EXPRESSION %prec NNOT {
        $$ = ast_node_new_op('!', 0, $2);
        DEBUG("    (Parser Responds: Found 'unary !' expresson)\n");
    }
    | LPAREN EXPRESSION RPAREN %prec SUBEXP {
        DEBUG("    [Parser Responds: Found '(subexpresson)' expresson]\n");
        $$ = $2;
    }
    | ERROR {
        YYABORT;
    }
;

%%

