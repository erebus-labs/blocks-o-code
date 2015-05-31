/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.tab.h"
#include "parse.lex.h"
#include "eval.h"
#include "globals.h"

/* Functions ***************************/
int abc_parse() {
    int parsesuccess = yyparse();

    #ifdef PRINTDEBUGINFO
        if (!parsesuccess) {
            DEBUG("\n");
            DEBUG("ABC says: Starting Interpretation...\n");
        }
    #endif

    if (!g_interactive_quit) {
        eval_expr(g_ast_root);
    }

    return parsesuccess;
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
        g_source_file = stdin;
    }

    // Call the parser
    yyin = g_source_file;
    int parsesuccess = 0;
    if (g_source_file == stdin) {
        g_interactive_quit = 0;
        while (g_interactive_quit != 1) {
            g_reset_globals();
            parsesuccess = abc_parse();
            if (g_interactive_quit > 1) {
                g_interactive_quit = 0;
            }
        }
    } else {
        parsesuccess = abc_parse();
    }

    return !parsesuccess;
}

