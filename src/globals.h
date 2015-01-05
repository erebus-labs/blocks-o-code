/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Greg's proposal for syntax/symantics

(C)2014-2015 Erubus Labs
* For internal use only (subject to change)

****************************************/

/// \file globals.h
/// \brief A nice place to put stuff

#ifndef ABC_GLOBALS_H
#define ABC_GLOBALS_H

#ifdef PRINTDEBUGINFO
#define DEBUG(...) fprintf(stderr, ">>> " __VA_ARGS__);
#else
#define DEBUG(...) ;
#endif

#endif
