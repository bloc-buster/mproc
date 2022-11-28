// -------------------------------------------------------------------------
// carriers.h 
//
// written by Sharlee Climer, August 2008
//
// ------------------------------------------------------------------------

#ifndef _CARRIERS_H
#define _CARRIERS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "timer.h"

const int QUIET = 1;  // set to one to eliminate output to screen
const int VERBOSE = 0;  // set to one to display maximum output to screen

const int MAX_NUM_INDIVIDUALS = 100000; // maximum number of individuals
const int MAX_NUM_SNPs = 10000000; // maximum number of SNPs
const int MAX_SIZE = 1000; // maximum size of cluster to be tested
const float TOL = 0.000001; // tolerance

inline void warning(const char* p) { fprintf(stderr,"Warning: %s \n",p); }
inline void fatal(const char* string) {fprintf(stderr,"\nFatal: %s\n\n",string); exit(1); }

#endif
 
