/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Tyler's proposal for syntax/symantics

(C)2014 Erubus Labs
* For internal use only (subject to change)

****************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.tab.h"
#include "eval.h"

/* Globals *****************************/
double globals[6] = {0, 1, 2, -1, 3.141592653589793, 10};
FILE* g_source_file;

/* Functions ***************************/
/* The lexer for flex */
int yylex(void) {
    int c;

    /* Skip tabs and spaces */
    while ((c = fgetc(g_source_file)) == ' ' || c == '\t') {
        continue;
    }

    /* Skip comments */
    if (c == '#') {
        while ((c = fgetc(g_source_file)) != '\n') {
            continue;
        }
        // c = fgetc(g_source_file);
    }

    /* Process Numbers */
    if (c == '.' || isdigit (c)) {
        ungetc(c, g_source_file);
        fscanf(g_source_file, "%lf", &yylval.SEMDouble);
        return NUM;
    }

    /* Process Variable */
    if ((c >= 'A') && (c <= 'F')) {
        yylval.SEMChar = c;
        return VAR;
    }

    /* Return end-of-input.  */
    if (c == EOF) {
        return 0;
    }

    /* Return a single char.  */
    return c;
}

/* Called when flex encounters an error */
void yyerror(char const *s) {
    fprintf(stderr, "ERROR: %s\n", s);
}

/* Main ********************************/
int main(int argc, const char** argv) {
    // Simple argparse
    if (argc > 1) {
        if (!strcmp(argv[1],"-h") ||
            !strcmp(argv[1],"--help"
        )) {
            // Display README file when -h is given
            #include "readme.xxd"
            readme_md[readme_md_len] = 0;
            printf("%s\n", readme_md);
            return 0;
        }
        if (!(g_source_file = fopen(argv[1], "r"))) {
            // Open file to scan
            fprintf(stderr, "ERROR: Could not open file: %s\n", argv[1]);
            return 1;
        }
    } else {
        // If no argument is provided, read from stdin
        g_source_file = stdin;
    }

    // Call the flex-generated parser
    yyparse();
    eval_expr(g_ast_root);
    return 0;
}

