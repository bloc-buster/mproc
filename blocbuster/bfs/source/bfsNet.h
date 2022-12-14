// -------------------------------------------------------------------------
// bfsNet.h -   Header file for breadth-first search of sparse networks
//
// written by Sharlee Climer, October 2007
//
// ------------------------------------------------------------------------

#ifndef _BFSNET_H
#define _BFSNET_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include "timer.h"

const int QUIET = 1;  // set to one to eliminate output to screen
const int VERBOSE = 0;  // set to one to display maximum output to screen
const int DIRECTED = 0; // set to one for directed graph

inline void warning(char* p) { fprintf(stderr,"Warning: %s \n",p); }
inline void fatal(char* string) {fprintf(stderr,"Fatal: %s\n",string);
                                 exit(1); }

#endif
