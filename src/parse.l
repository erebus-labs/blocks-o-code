/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/


%{
    #include <stdio.h>
    #include <string.h>

    #include "parse.tab.h"
    #include "globals.h"

    int yycolnum = 1;

    #define YY_USER_ACTION {yylloc.first_line = yylineno;   \
        yylloc.first_column = yycolnum;                     \
    }

    extern int errorline;
    extern int errorcolumn;
    extern char* errortext;
    extern int unrecognized;

    unsigned int nest_level = 1;
    char expects_condition = 0;
    void print_nest() {
        switch (expects_condition) {
        case 0:
            for (int i = 0; i < nest_level; i++) {
                putchar('>');
            }
            break;
        case 1:
            printf("While what?");
            break;
        case 2:
            printf("If what?");
            break;
        }
        putchar(' ');
    }

%}

%option noyywrap
%option yylineno

%%

#[^\n]*\n {
    /* Ignore Comments */
    yycolnum = 1;
    if (g_source_file == stdin) {
        // Only execute (by returning EOF) when on the global scope
        if (nest_level == 1) {
            DEBUG("Lexer says: Found a comment!\n");
            return EOF;
        }
        print_nest();
    }
    }

[ \t] {
    /* Ignore Whitespace */
    yycolnum++;
    }

\n {
    yycolnum = 1;
    if (g_source_file == stdin) {
        // Only execute (by returning EOF) when on the global scope
        if (nest_level == 1) {
            DEBUG("Lexer says: Accepting input\n");
            return EOF;
        }
        print_nest();
    }
    }

[0-9]+\.[0-9]+ {
    /* Grab Number Literals with decimals */
    sscanf(yytext, "%lf", &yylval.SEMDouble);
    yycolnum += yyleng;
    DEBUG("Lexer says: Found Literal: %g\n", yylval.SEMDouble);
    return NUM;
    }

[0-9]+ {
    /* Grab Number Literals without decimals */
    sscanf(yytext, "%lf", &yylval.SEMDouble);
    DEBUG("Lexer says: Found Literal: %g\n", yylval.SEMDouble);
    yycolnum += yyleng;
    return NUM;
    }

(?i:while) {
    /* Statements */
    /* Case insensitive */
    nest_level++;
    expects_condition = 1;
    yycolnum+=6;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return WHILE;
    }
(?i:end\ while)|(?i:endwhile) {
    if (nest_level > 0) nest_level--;
    yycolnum+=9;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return END_WHILE;
    }
(?i:if) {
    nest_level++;
    expects_condition = 2;
    yycolnum+=2;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return IF;
    }
(?i:else) {
    yycolnum+=4;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return ELSE;
    }
(?i:end\ if)|(?i:endif) {
    if (nest_level > 0) nest_level--;
    yycolnum+=6;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return END_IF;
    }
(?i:say)|(?i:print) {
    yycolnum+=3;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return SAY;
    }
(?i:output) {
    yycolnum+=6;
    DEBUG("Lexer says: Found Statement: %s\n", yytext);
    return OUTPUT;
    }


(?i:a) {
    /* Variables */
    /* Case insensitive */
    yycolnum++;
    yylval.SEMChar = 0;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:b) {
    yycolnum++;
    yylval.SEMChar = 1;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:c) {
    yycolnum++;
    yylval.SEMChar = 2;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:x) {
    yycolnum++;
    yylval.SEMChar = 3;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:y) {
    yycolnum++;
    yylval.SEMChar = 4;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:z) {
    yycolnum++;
    yylval.SEMChar = 5;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:sum) {
    yycolnum+=3;
    yylval.SEMChar =  6;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }
(?i:count) {
    yycolnum+=5;
    yylval.SEMChar =  7;
    DEBUG("Lexer says: Found Variable: %s\n", yytext);
    return VAR;
    }

"+=" {
    /* Assignment Operators */
    yycolnum+=2;
    DEBUG("Lexer says: Found Compound Assignment: %s\n", yytext);
    return AS_PLUS;
    }
"-=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Compound Assignment: %s\n", yytext);
    return AS_MINUS;
    }
"*=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Compound Assignment: %s\n", yytext);
    return AS_MULT;
    }
"/=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Compound Assignment: %s\n", yytext);
    return AS_DIV;
    }
"^=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Compound Assignment: %s\n", yytext);
    return AS_POW;
    }
"%=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Compound Assignment: %s\n", yytext);
    return AS_MOD;
    }

"+" {
    /* Aritmetic Operators */
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_PLUS;
    }
"-" {
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_MINUS;
    }
"*" {
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_MULT;
    }
"/" {
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_DIV;
    }
"^" {
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_POW;
    }
"%" {
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_MOD;
    }

"==" {
    /* Conditional Operators */
    yycolnum+=2;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_EQ;
    }
"=" {
    yycolnum++;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_EQ;
    }
"<" {
    yycolnum++;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_LT;
    }
">" {
    yycolnum++;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_GT;
    }
"<=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_LE;
    }
">=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_GE;
    }
"!=" {
    yycolnum+=2;
    DEBUG("Lexer says: Found Conditional: %s\n", yytext);
    return EX_NE;
    }
"!" {
    yycolnum++;
    DEBUG("Lexer says: Found Operator: %s\n", yytext);
    return EX_CLAIMATION;
    }
"(" {
    /* Parens */
    yycolnum++;
    return LPAREN;
    }
")" {
    yycolnum++;
    return RPAREN;
    }

<<EOF>> {
    DEBUG("Lexer says: Found EOF%s\n", yytext);
    return EOF;
    }

(?i:quit)|(?i:exit)|"q" {
    /* Interactive Commands */
    if (g_source_file == stdin) {
        expects_condition = 0;
        g_interactive_quit = 1;
        return INTERACTIVE_QUIT;
    }
    /* Dont recognize this if not in interactive mode */
    errorline = yylineno;
    errorcolumn = yycolnum;
    errortext = yytext;
    unrecognized = 1;
    return ERROR;
    }

(?i:cancel)|(?i:forget) {
    if (g_source_file == stdin) {
        g_interactive_quit = 2;
        if (nest_level > 0) nest_level--;
        DEBUG("Lexer Says: Cancelling...\n");
        YY_FLUSH_BUFFER;
        return INTERACTIVE_QUIT;
    }
    /* Dont recognize this if not in interactive mode */
    errorline = yylineno;
    errorcolumn = yycolnum;
    errortext = yytext;
    unrecognized = 1;
    return ERROR;
    }

. {
    /* Anything else is an error */
    errorline = yylineno;
    errorcolumn = yycolnum;
    errortext = yytext;
    unrecognized = 1;
    return ERROR;
    }

%%

void yyerror(char const* error) {
    if (g_interactive_quit == 0) {
        if (unrecognized) {
            #ifdef PRINTDEBUGINFO
            fprintf(stderr, "\n");
            #endif
            fprintf(stderr, "ERROR: Unexpected Token(s): %s\n", errortext);
            fprintf(stderr, "       on line %d, column %d\n", errorline, errorcolumn);
            #ifdef PRINTDEBUGINFO
            DEBUG("Parser and Lexer are too confused to finish their jobs...\n")
            #endif
        } else {
            #ifdef PRINTDEBUGINFO
            fprintf(stderr, "\n");
            DEBUG("Parser and Lexer are too confused to finish their jobs...\n")
            #else
            fprintf(stderr, "ERROR: %s\n", error);
            #endif
        }
        g_interactive_quit = 1;
    }
}

