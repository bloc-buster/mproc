// -------------------------------------------------------------------------
// keepHighWt.h -   Header file for retaining high weight edges
//
// written by Sharlee Climer, June 2011
//
// ------------------------------------------------------------------------

#ifndef _KEEPHIGHWT_H
#define _KEEPHIGHWT_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <algorithm>
#include "timer.h"


const int QUIET = 1;  // set to one to eliminate output to screen
const int VERBOSE = 0;  // set to one to display maximum output to screen

const float TOL = 0.00000001; // tolerance


inline void warning(char* p) { fprintf(stderr,"Warning: %s \n",p); }
inline void fatal(char* string) {fprintf(stderr,"Fatal: %s\n",string);
                                 exit(1); }

#endif
