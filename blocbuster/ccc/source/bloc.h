// -------------------------------------------------------------------------
// bloc.h -   Header file for BlocBuster
//
// Release v1.0
// June 2014
// written by Sharlee Climer
//
// ------------------------------------------------------------------------

#ifndef _BLOC_H
#define _BLOC_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "timer.h"

const int ROWS_R_SNPS = 0; // set to 1 if each row (0 if each column) represents a SNP

const int QUIET = 1;  // set to one to eliminate output to screen (Boolean)
const int VERBOSE = 0;  // set to one to display maximum output to screen (Boolean)

const int PRINTGML = 1; // print sparse network to .gml file (Boolean)
const int PRINTNUMEDGES = 0; // print number of edges to 'numEdges.txt'
const int PRINT_EDGE_IDS = 0; // print edge ID numbers to 'edgeList.txt'

const int MISSING_SYMBOL = -1; // ASCII value of customized symbol for missing data

const int TWONODE = 1; // create a network with two nodes for each SNP (Boolean)
const int PRINTFREQ = 0; // set to 1 to print out frequencies to "temp.freq" (Boolean)
const int FREQ = 1; // use frequency information in correlation value (Boolean)
const float FREQWT = 1.5; // weight used for frequency factor (1.5)

const float NOMISS = 0.5; // minimum fraction of individuals without missing relationships
                          // if too many missing, a warning message is printed
const int WARN_MISS = 0; // set to 1 to print these warning messages (Boolean)

const int SCREEN_INPUT = 0; // set to 1 to be prompted for # of header rows/cols (Boolean)
const int LOG_FILE = 1; // set to 1 to record screen output to log file (Boolean)

// the following can be adjusted if needed
const int MAX_NUM_EDGES = 10000000; // maximum number of edges output
const int MAXNUMHEADERS = 200; // maximum number of header rows or columns
const int MAX_NUM_INDIVIDUALS = 1000000; // maximum number of individuals
const int MAX_NUM_SNPS = 10000000; // maximum number of SNPs
const double TOL = 0.00001; // tolerance



inline void warning(const char* p) { fprintf(stderr,"Warning: %s \n",p); }
inline void fatal(const char* string) {fprintf(stderr,"\nFatal: %s\n\n",string); exit(1); }

#endif
