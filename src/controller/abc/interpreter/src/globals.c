/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/

#include <stdio.h>

/* Globals *****************************/
double globals[8]         = {0, 1, 2, -1, 3.141592653589793, 10, 0, 0};
double globals_restore[8] = {0, 1, 2, -1, 3.141592653589793, 10, 0, 0};
FILE* g_source_file;
int g_interactive_quit;

void g_reset_globals() {
    for (int x = 0; x < 8; x++) {
        globals[x] = globals_restore[x];
    }
}
