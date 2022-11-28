// -------------------------------------------------------------------------
// randomize.h -   Header file for randomizer
//
// written by Sharlee Climer, January 2011
//
// ------------------------------------------------------------------------

#ifndef _RANDOMIZE_H
#define _RANDOMIZE_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>

#include "timer.h"

const int QUIET = 1;  // set to one to eliminate output to screen
const int VERBOSE = 0;  // set to one to display maximum output to screen

const int SIZESTRNG = 20; // size of string allocated for each input data
const int SIZESTRNGHEAD = 500; // size of string allocated for each header string
const int MAXNUMHEADERS = 200; // maximum number of header rows or columns
const int SCREEN_INPUT = 0; // set to 1 to be prompted for # of header rows/cols

const double TOL = 0.00001; // tolerance

inline void warning(const char* p) { fprintf(stderr,"Warning: %s \n",p); }
inline void fatal(const char* string) {fprintf(stderr,"\nFatal: %s\n\n",string); exit(1); }

#endif
