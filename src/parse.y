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
    double globals[6];
%}

/* Types **********************/

/* Default value for literals is a double */
%define api.value.type {double}
%token NUM

/* One of 6 variables [A-F]  */
%token VAR


/* Precedence *****************/
%left '-' '+'                   /* Arithmetic Operators     */
%left '*' '/'                   /*                          */
%precedence NNEG                /* Unary Numerical Negation */
%right '^' '%'                  /* Arithmetic Operators     */
%right '!'                      /* Unary Logical Negation   */
%left '=' '>' '<' 'l' 'g' '~'   /* Comparision Operators    */
%right 'p'                      /* Print Action             */
%left ':'                       /* Assignment Operator      */
%precedence EVAR                /* Empty variable statement */


/* Semantics ******************/
%%

input:
      %empty
    | input statement
;

statement:
      '\n'
    | action ';'
    | action '\n'
;

action:
    'p' exp {
        printf("%.10g\n", $2);
    }
    | VAR ':' exp {
        int dex = $1 - 'A';
        globals[dex] = $3;
        printf("Assigning variable '%c' the value: %.10g\n", (char)$1, $3);
    }
    | exp {
        printf("Expression Result: %.10g\n", $1);
    }
;

exp:
    NUM {
         $$ = $1;
    }
    | VAR %prec EVAR {
        int dex = $1 - 'A';
        $$ = globals[dex];
    }
    | exp '+' exp {
        $$ = $1 + $3;
    }
    | exp '-' exp {
        $$ = $1 - $3;
    }
    | exp '*' exp {
         $$ = $1 * $3;
    }
    | exp '/' exp {
        $$ = $1 / $3;
    }
    | '-' exp  %prec NNEG {
        $$ = -$2;
    }
    | '!' exp {
        $$ = !$2;
    }
    | exp '^' exp {
        $$ = pow($1, $3);
    }
    | exp '%' exp {
        $$ = (double)((int)$1 % (int)$3);
    }
    | exp '=' exp {
        $$ = $1 == $3;
    }
    | exp '>' exp {
        $$ = $1 > $3;
    }
    | exp '<' exp {
        $$ = $1 < $3;
    }
    | exp 'l' exp {
        $$ = $1 <= $3;
    }
    | exp 'g' exp {
        $$ = $1 >= $3;
    }
    | exp '~' exp {
        $$ = $1 != $3;
    }
    | '(' exp ')' {
        $$ = $2;
    }
;

%%

