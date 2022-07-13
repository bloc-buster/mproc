/****************************************************************************
*
*	bloc.cpp:	Code for quantifying correlations between alleles 
*                       for a group of individuals.  
*      
*                       Release v1.0
*                       June 2014
*                       Sharlee Climer
*
*                       
****************************************************************************/
  

#include "bloc.h"

using namespace std;

void format(char*, char**, char**, char**, double**, double**, int, int, int, int, int, int, char*, int, int, int); // read in and format input data 

void checkConstants(); // check validity of constants in bloc.h


int main(int argc, char ** argv)
{
  if (argc != 8)
    fatal("Usage:\n\n   ccc input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols\n\n");  

  timer t;
  t.start("Timer started.");

  cout << "\nCommand line arguments: \n\t";
  for (int i = 0; i < argc; i++)
	cout << argv[i] << " ";
  cout << "\n" << endl;

  checkConstants(); // check validity of constants defined in bloc.h

  FILE *logfile; // log file that records screen output

  char logfileName[200]; // hold name of logfile
  char base[100]; // base for logfile name
  const char logSuffix[] = ".bloc.log"; // suffix for logfile name

  // determine name for logfile
    for (int i = 0; i < 100; i++) {
      base[i] = argv[2][i]; // copy output file name
      if (base[i] == '\0') {
	if(i < 5)
	  fatal("Expected output file name to have '.gml' suffix");
	
	base[i-4] = '\0'; // set end of string to not include '.gml'
	break;
      }
    }
  
    sprintf(logfileName, "%s%s", base, logSuffix); // string together file name

  if(LOG_FILE) 
    cout << "Screen output will be recorded in '" << logfileName <<"'.\n" << endl;

  if ((logfile = fopen(logfileName, "w")) == NULL)
      fatal("Log file could not be opened.\n");

  if (LOG_FILE) {
    fprintf(logfile, "\nCommand line arguments: \n\t");
    for (int i = 0; i < argc; i++)
      fprintf(logfile, "%s ", argv[i]);
    fprintf(logfile, "\n\n");
  }

  float thresh = atof(argv[3]);

  // changed default threshold, so this is just a reminder
  if ((thresh < 0.7-TOL) || (thresh > 0.7+TOL))
    warning("Default threshold value is 0.7.");

  //if(!TWONODE)
  //fatal("Check results are correct when only one node is output per SNP.");

  char strng[200]; // temporary string storage

  // determine if rows or columns represent SNPs
  if ((ROWS_R_SNPS != 0) && (ROWS_R_SNPS != 1))
    fatal("Invalid value for ROWS_R_SNPS in 'bloc.h'.");

  if (ROWS_R_SNPS == 0)
    cout << "\n***Important: Assumed rows represent individuals and \n   columns represent SNPs in input file.***\n\n" << endl;

  if (ROWS_R_SNPS == 1)
    cout << "\n***Important: Assumed rows represent SNPs and \n   columns represent individuals in input file.***\n\n" << endl;

  if(LOG_FILE) {
    if(ROWS_R_SNPS == 0)
      fprintf(logfile, "\n***Important: Assumed rows represent individuals and \n   columns represent SNPs in input file.***\n\n\n");

  if(ROWS_R_SNPS == 1)
      fprintf(logfile, "\n***Important: Assumed rows represent SNPs and \n   columns represent individuals in input file.***\n\n\n");
  }

  if(TWONODE) {
    cout << "Each SNP will be represented by two nodes in output graph." << endl;
    
    if(LOG_FILE)
      fprintf(logfile, "Each SNP will be represented by two nodes in output graph.\n");
  }

  if(!TWONODE) {
    cout << "Each SNP will be represented by one node in output graph." << endl;
    
    if(LOG_FILE)
      fprintf(logfile, "Each SNP will be represented by one node in output graph.\n");
  }

  if(FREQ) {
	cout << "Frequencies used in computations with a weight of " << FREQWT << ".\n" << endl;
  if(LOG_FILE)
    fprintf(logfile, "Frequencies used in computations with a weight of %f.\n\n", FREQWT);
  }

  int numInd = atoi(argv[4]); // number of individuals
  int numSnps = atoi(argv[5]);  // number of genes
  int numNodes = 2 * numSnps; // number of nodes

  cout << numInd << " individuals and " << numSnps << " SNPs in entire input file." << endl;
  cout << "Threshold of " << thresh << " used." << endl;

  // set default values to compute entire set of SNPs
  // will reduce by one later
  int start1 = 1;  
  int end1 = numSnps;
  int start2 = 1;
  int end2 = numSnps;

  // change values if different values given on command line
  if(argc == 12) { 
	start1 = atoi(argv[8]);
	end1 = atoi(argv[9]);
	start2 = atoi(argv[10]);
	end2 = atoi(argv[11]);
  }

  if (argc == 12) {
    cout << "First SNP will range between " << start1 << " and " << end1 << endl;
    cout << "Second SNP will range between " << start2 << " and " << end2 << endl;

  if(LOG_FILE)
    fprintf(logfile,"First SNP will range between %d and %d;\nsecond SNP will range between %d and %d.\n\n",start1, end1, start2, end2);
  }

  // compute only upper diagonal edges, so check that first set has starting number that 
  // is no more than the starting point for second set
  if(start1 > start2) 
	fatal("First SNP start number can't be greater than second SNP start number (upper diagonal is computed)");

  // check values are valid
  if((start1 < 1) || (start2 < 1))
	fatal("SNP start number is less than 1");

  if((end1 < start1) || (end2 < start2))
	fatal("Start SNP number is less than end SNP number");

  if((end1 > numSnps) || (end2 > numSnps))
	fatal("End SNP number is greater than number of SNPs");

  // check to see if exclusion of lower diagonal will exclude something that might be intended
  if(end1 > end2)
	fatal("Only upper diagonal of matrix is computed.\nComputations won't be made for the SNP1 values that are greater than the highest SNP2 value.");

  // adjust values for matrix indices that range from 0 to numSnps - 1
  start1--;
  end1--;
  start2--;
  end2--;

  // determine number of SNPs in each set 
  int numSnps1 = end1 - start1 + 1; 
  int numSnps2 = end2 - start2 + 1;

  if ((numSnps1 < 1) || (numSnps2 < 1))
    fatal("Too few SNPs selected.");

  // determine number of header rows and columns
  // find number of header rows and columns
  int numheadcols = -1;
  int numheadrows = -1;

  if (SCREEN_INPUT) { // prompt use to provide number of header rows/cols
    cout << "\n\nEnter number of header rows in data: ";
    cin >> numheadrows;
    
    if ((numheadrows < 0) || (numheadrows > MAXNUMHEADERS)) {
      cout << numheadrows << endl;
      fatal("Invalid number of header rows.");
    }
    
    cout << "Enter number of header columns in data: ";
    cin >> numheadcols;
    
    if ((numheadcols < 0) || (numheadcols > MAXNUMHEADERS)) {
      cout << numheadcols << endl;
      fatal("Invalid number of header columns.");
    }
  }

  if (!SCREEN_INPUT) {
    numheadrows = atoi(argv[6]); // assign values from command line
    numheadcols = atoi(argv[7]);
  }

  if(LOG_FILE) 
    fprintf(logfile, "%d individuals and %d SNPs in entire dataset.\nThreshold of %f used.\n", numInd, numSnps, thresh);

  if(PRINT_EDGE_IDS) {
    cout << "\nIMPORTANT: Edge IDs will be appended to 'edgeList.txt' for each edge produced.\n\tThe edge ID = (numNodes * i) + j, where i = source and j = target.\n" << endl;
    if(LOG_FILE)
      fprintf(logfile, "\nIMPORTANT: Edge IDs will be appended to 'edgeList.txt' for each edge produced.\n\tThe edge ID = (numSNPs * i) + j, where i = source and j = target.\n\n");
  }

  if (numInd > MAX_NUM_INDIVIDUALS)
	fatal("Too many individuals.  Fix header file.");
  if (numSnps > MAX_NUM_SNPS)
	fatal("Too many SNPs.  Fix header file.");
  if ((numInd < 2) || (numSnps < 2))
	fatal("Too few individuals or SNPs");

  if (VERBOSE) 
	cout << numSnps1 << " SNPs in first set, " << numSnps2 << " SNPs in second set." << endl;

  int printFreq = 0; // flag to print frequencies out to 'temp.freq'
  if (PRINTFREQ && (argc == 8))
    printFreq = 1; // set flag to print frequencies only if full data set being computed

  // allocate data memory
  char ** data1;  // hold first set of genotypes
  char ** data2;  // hold second set of genotypes

  if ((numSnps1 < 1) || (numSnps2 < 1))
	fatal("Number of SNPs in set is less than 1");

  if (((data1 = new char* [numSnps1]) == NULL) || ((data2 = new char* [numSnps2]) == NULL))
    fatal("Memory not allocated");

  for (int i = 0; i < numSnps1; i++)
    if ((data1[i] = new char[numInd]) == NULL)
      fatal("Memory not allocated");

  for (int i = 0; i < numSnps2; i++)
    if ((data2[i] = new char[numInd]) == NULL)
      fatal("Memory not allocated");

  // initialize values to '4' as valid values are 0, 1, 2, or 3
  for (int i = 0; i < numSnps1; i++)
    for (int j = 0; j < numInd; j++)
      data1[i][j] = '4';

  for (int i = 0; i < numSnps2; i++)
    for (int j = 0; j < numInd; j++)
      data2[i][j] = '4';

  // allocate memory for storing frequency information
  double **freq1; // hold frequency values for alleles for first SNP set
  double **freq2; // hold frequency values for alleles for second SNP set  

  if ((freq1 = new double*[numSnps1]) == NULL) 
    fatal("memory not allocated");

  for (int i = 0; i < numSnps1; i++)
    if ((freq1[i] = new double[2]) == NULL)
      fatal("memory not allocated");  

  if ((freq2 = new double*[numSnps2]) == NULL) 
    fatal("memory not allocated");

  for (int i = 0; i < numSnps2; i++)
    if ((freq2[i] = new double[2]) == NULL)
      fatal("memory not allocated");

  // initialize frequency values to zero
  for (int i = 0; i < numSnps1; i++)
    for (int j = 0; j < 2; j++)
      freq1[i][j] = 0;

  for (int i = 0; i < numSnps2; i++)
    for (int j = 0; j < 2; j++)
      freq2[i][j] = 0;

  // allocate memory for holding alleles for each SNP
  char **allele;  

  if ((allele = new char*[numSnps]) == NULL)
    fatal("memory not allocated");
  for (int i = 0; i < numSnps; i++)
    if ((allele[i] = new char[2]) == NULL)
      fatal("memory not allocated");

  // initialize alleles to zero
  for (int i = 0; i < numSnps; i++)
    for (int j = 0; j < 2; j++)
      allele[i][j] = '0';

  // read in and format input data (close logfile first)
  fclose(logfile);

  // format function will assemble data in the matrices and 
  // writes out the number of missing values
  format(argv[1], data1, data2, allele, freq1, freq2, numSnps, numInd, start1, end1, start2, end2, logfileName, numheadrows, numheadcols, printFreq); 

  //reopen logfile
  if ((logfile = fopen(logfileName, "a")) == NULL)
      fatal("Log file could not be opened.\n");

  cout << "\nComputing CCC values..." << endl;

  if(LOG_FILE)
    fprintf(logfile, "\nComputing CCC values...\n");

  // write out nodes to output file
  FILE *output;
  FILE *edgefile; // use for edge IDs if PRINT_EDGE_IDS is set to 1

  if ((output = fopen(argv[2], "w")) == NULL)
      fatal("Output file could not be opened.\n");

  if (TWONODE) { // 2 nodes for each SNP
    fprintf(output, "Graph with %d nodes. \ngraph\n[\n", numNodes);
    for (int j = 1; j <= numNodes; j++)
      fprintf(output, "\tnode \n\t[\n\tid %d \n\t]\n", j);
  }
  
  else {
    fprintf(output, "Graph with %d nodes. \ngraph\n[\n", numSnps);
    for (int j = 1; j <= numSnps; j++)
      fprintf(output, "\tnode \n\t[\n\tid %d \n\t]\n", j);
  }
  
  // compute correlations and output edges

  float tally[4][4]; // tally number of each of 16 possible combinations
  float maxBloc = 0.0; // initialize for finding max and min values
  float minBloc = 1.0; 
  long int numEdges = 0; // tally number of edges printed out
  float minNoMissing = (float)numInd * NOMISS; // minimum of no missing relationships

  // adjust threshold to equal unscaled and unshifted value
  thresh = thresh / 4.5; // divide by 4.5 to get R_ij * ff_i * ff_j value

  // tally pairwise correlations
  for (int i = 0; i < numSnps1; i++) // start with each SNP in first set
    for (int j = 0; j < numSnps2; j++)  // pair with each SNP in second set

      if (start1+i < start2+j) { // only compute upper diagonal of matrix

      // initialize values
      for (int row = 0; row < 4; row++)
	for (int col = 0; col < 4; col++)
	  tally[row][col] = 0;

      // add up number of individuals with each relationship      
      for (int k = 0; k < numInd; k++)
	tally[data1[i][k]][data2[j][k]]++; 

      // count how many individuals have no missing data
      int noMissing = 0;
      
      for (int row = 0; row < 3; row++) 
	for (int col = 0; col < 3; col++)
	  noMissing += (int)tally[row][col];
      
      //cout << noMissing << " relationships" << endl;
      
      // print out warning if flag set and too few relationships
      if(WARN_MISS && (noMissing < minNoMissing)) {
	cout << "SNPs " << start1+i+1 << " and " << start2+j+1 << " have " ;
	cout << noMissing << " relationships without missing data." << endl;
	warning ("Correlation is based on too few relationships.");

	if(LOG_FILE)
	  fprintf(logfile, "SNPs %d and %d have %d relationships without missing data.\nWarning: Correlation is based on too few relationships.\n\n", start1+i+1, start2+j+1, noMissing);
      }

      // adjust proportionate contributions of each relationship
      tally[1][1] /= 4.0; // both heterozygous
      tally[0][1] /= 2.0; // one heterozygous, the other homozygous
      tally[1][0] /= 2.0; // one heterozygous, the other homozygous
      tally[1][2] /= 2.0; // one heterozygous, the other homozygous
      tally[2][1] /= 2.0; // one heterozygous, the other homozygous

      // compute four relationship values
      // both alleles are lowest alphabetically
      float ll = tally[0][0] + tally[0][1] + tally[1][0] + tally[1][1];

      // first allele lowest, second highest
      float lh = tally[0][1] + tally[0][2] + tally[1][1] + tally[1][2];

      // first allele highest, second lowest
      float hl = tally[1][0] + tally[1][1] + tally[2][0] + tally[2][1];

      // both alleles are highest alphabetically
      float hh = tally[1][1] + tally[1][2] + tally[2][1] + tally[2][2];

      if (0) 
	cout << start1+i+1 << ", " << start2+j+1 << ": " << "ll = " << ll << ", lh = " << lh << ", hl = " << hl << ", hh = " << hh << endl;

      // find average by dividing by number of individuals
      ll /= (float)noMissing;
      lh /= (float)noMissing;
      hl /= (float)noMissing;
      hh /= (float)noMissing;

      if (0) { // temporary check to see if values add to one
	float tempSum = ll+lh+hl+hh;
	if ((tempSum > 1.0+TOL) || (tempSum < 1.0-TOL)) {
	  cout << tempSum << endl;
	  fatal("Values do not add to one");
	}
      }

      if (0) 
	cout << start1+i+1 << ", " << start2+j+1 << ": " << "ll = " << ll << ", lh = " << lh << ", hl = " << hl << ", hh = " << hh << endl;

      // multiply by frequency factors
      if (FREQ) {
	ll *= freq1[i][0] * freq2[j][0]; // multiply by two frequency factors
	lh *= freq1[i][0] * freq2[j][1]; 
	hl *= freq1[i][1] * freq2[j][0]; 
	hh *= freq1[i][1] * freq2[j][1]; 
      }

      if (VERBOSE) 
	cout << start1+i+1 << ", " << start2+j+1 << ": " << "ll = " << ll * 4.5 << ", lh = " << (lh * 4.5)  << ", hl = " << (hl * 4.5)  << ", hh = " << (hh * 4.5) << endl;

      float max = ll; // find maximum value
      if (lh > max)
	max = lh;
      if (hl > max)
	max = hl;
      if (hh > max)
	max = hh;

      if(VERBOSE)
	cout << "Max = " << (max * 4.5) << endl;

      // update maximum and minimum values found for data set
      if (max > maxBloc)
	maxBloc = max;

      if (max < minBloc)
	minBloc = max;

      // check that values are valid
      if((ll > 1.0 + TOL) || (ll < 0.0 - TOL))
	warning("Invalid value computed for ll relationship.");

      if((lh > 1.0 + TOL) || (lh < 0.0 - TOL))
	warning("Invalid value computed for lh relationship.");

      if((hl > 1.0 + TOL) || (hl < 0.0 - TOL))
	warning("Invalid value computed for hl relationship.");

      if((hh > 1.0 + TOL) || (hh < 0.0 - TOL))
	warning("Invalid value computed for hh relationship.");

      // print out significant edges
      if (!TWONODE) // just one possible edge to print out
	if (max > thresh - TOL) {

	  float weight = (max * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL))
	    fatal("Invalid CCC value");

	  fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+1, weight);
	  numEdges++;

	  if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+1)*numNodes + (start2+j+1));
	    fclose(edgefile);
	  }
	}

      if (TWONODE) {
	if(ll > thresh - TOL) { 

	  float weight = (ll * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");

	    if(LOG_FILE)
	      fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  }

	  fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+1, weight);
	  numEdges++;	

	  if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+1)*numNodes + (start2+j+1));
	    fclose(edgefile);
	  }
	}

	if(lh > thresh - TOL) { 
	  
	  float weight = (lh * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");
	    
	    
	    if(LOG_FILE)
	      fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  }

	  fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+numSnps+1, weight);
	  numEdges++;

	  if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+1)*numNodes + (start2+j+numSnps+1));
	    fclose(edgefile);
	  }
	}

	if(hl > thresh - TOL) { 
	  
	  float weight = (hl * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");

	    if(LOG_FILE)
	      fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  } 

	  fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+numSnps+1, start2+j+1, weight);
	  numEdges++;

	  if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+numSnps+1)*numNodes + (start2+j+1));
	    fclose(edgefile);
	  }
	}

	if(hh > thresh - TOL) { 
	  
	  float weight = (hh * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");
	    
	    if(LOG_FILE)
	      fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  }

	  fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+numSnps+1, start2+j+numSnps+1, weight);
	  numEdges++;

	  if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+numSnps+1)*numNodes + (start2+j+numSnps+1));
	    fclose(edgefile);
	  }
	}
      }   
       
      // check that not too many edges are printed
      if(numEdges > MAX_NUM_EDGES)
	fatal("Too many edges printed out. Check MAX_NUM_EDGES in header file.");
    } // end of for (int j = i+1) loop

  fprintf(output, "]\n"); // print closing bracket

  fclose(output);

  if(PRINT_EDGE_IDS) {
    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
      fatal("'edgeList.txt' file could not be opened.\n");
    fprintf(edgefile, "\n");
    fclose(edgefile);
  }

  // rescale and shift the CCC values to range from 0 to 1
  minBloc = (minBloc * 4.5);
  maxBloc = (maxBloc * 4.5);
  thresh = (thresh * 4.5);

  // min can be less than 0 due to round-off error
  if (minBloc < 0)
	minBloc = 0;

  //cout << "\nCCCmax values range from " << minBloc << " to " << maxBloc << endl;
  cout << numEdges << " Custom correlations with values >= " << thresh << endl;

  if(LOG_FILE)
    fprintf(logfile, "\n%d Custom correlations with values >= %f.\n", numEdges, thresh);

  if (PRINTNUMEDGES) { // print number of edges to "numEdges.txt"
    FILE *edgeFile;

    // check if new file, if so print out header
    if ((edgeFile = fopen("numEdges.txt", "r")) == NULL) { 
      if ((edgeFile = fopen("numEdges.txt", "w")) == NULL)
	fatal("'numEdges.txt' file could not be opened.\n");

      fprintf(edgeFile,"NumEdges\tMaxCCC\n"); 
      fclose(edgeFile);
    }

  if ((edgeFile = fopen("numEdges.txt", "a")) == NULL)
    fatal("'numEdges.txt' file could not be opened.\n");

  fprintf(edgeFile,"%d\t%.5f\n",numEdges, maxBloc);
  fclose(edgeFile);
  }

  t.stop("\nTimer stopped.");
  cout << t << " seconds.\n" << endl;

  double compTime;
  compTime = t.timeVal();

  if(LOG_FILE)
    fprintf(logfile, "\nTimer stopped.\n%f seconds.\n", compTime);

  fclose(logfile);

  return 1;
}



void format(char* filename, char** data1, char** data2, char** allele, double** freq1, double** freq2, int numSnps, int numInd, int start1, int end1, int start2, int end2, char* logfileName, int numheadrows, int numheadcols, int printFreq) // read in and format input data
{
  if (!QUIET)
    cout << "\nReading in and formatting data...\n" << endl;

  FILE *input, *logfile;
  char strng[200]; // temporary string storage
  int ascii1; // temp storage for ascii value of first allele
  int ascii2; // temp storage for ascii value of second allele
  long int totalNumMissing1 = 0; // count total number of missing values in first set
  long int totalNumMissing2 = 0; // count total number of missing values in second set

  // determine number of SNPs in each set
  int numSnps1 = end1 - start1 + 1; 
  int numSnps2 = end2 - start2 + 1;

  // set number of rows and columns of data in input file
  int numInRows, numInCols;

  // if each row represents an individual (e.g. Plink format)
  if (ROWS_R_SNPS == 0) {
    numInRows = numInd;
    numInCols = numSnps;
  }

  // if each row represents a SNP (e.g. HapMap format)
  if (ROWS_R_SNPS == 1) {
    numInRows = numSnps;
    numInCols = numInd;
  }

  // open logfile
  if ((logfile = fopen(logfileName, "a")) == NULL)
      fatal("Log file could not be opened.\n");
  
  if(LOG_FILE)
    fprintf(logfile, "Assumed %d header rows and %d header columns in input file.\n\nReading in data...\n", numheadrows, numheadcols); 
  
  cout << "\nReading in data..." << endl;

  if ((input = fopen(filename, "r")) == NULL)
    fatal("Input file could not be opened.\n");

  // read in data and determine format  

  int space = 0; // set to 1 if space between alleles in input
  int slash = 0; // set to 1 if slash mark between alleles in input
  int format = 0; // set to 1 once format is determined

  if(0) { // print out names of individuals and terminate program
    fatal("Need to fix");
    long int numCols = 2*numSnps + numheadcols;
    for (int i = 0; i < numInd; i++)
      for (int j = 0; j < numCols; j++) {
	fscanf(input, "%s", strng); 
	if (j == 1)
	  cout << strng << endl;
      }
    exit(1);
  }

  // read in header rows and disregard
  for (int i = 0; i < numheadrows; i++)
    for (int j = 0; j < numheadcols + numInCols; j++)
      fscanf(input, "%s", strng); 

  // read in data
  for (int i = 0; i < numInRows; i++) {
    if (format)
      break; // already determined format

    for (int j = 0; j < numheadcols; j++) 
      fscanf(input, "%s", strng); // read in and disregard header columns

    for (int j = 0; j < numInCols; j++) {
      if(feof(input))
	fatal("Input file is missing data");

      fscanf(input, "%s", strng);
      //cout << i << ", " << j << ": " << strng << endl;

      ascii1 = strng[0]; // find ascii value of first char

      if ((ascii1 != 48) && (ascii1 != 78)) // not 'N' or '0'
	if(ascii1 != MISSING_SYMBOL) // not customized missing symbol
	if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
	
	  if ((int)strng[1] == 47) // second char is a '/'
	    slash = 1;
	  
	  else if ((int)strng[1] < 65) // second char is not a letter
	    space = 1; // space between chars
	  
	  format = 1;
	  break; // determined format
	}
    }
  }
  
  // reread file and determine alleles for each SNP

  rewind(input); // start over at beginning of file

  // read in header rows and disregard
  for (int i = 0; i < numheadrows; i++)
    for (int j = 0; j < numheadcols + numInCols; j++)
      fscanf(input, "%s", strng); 

  // read in data
  for (int i = 0; i < numInRows; i++) {
    for (int j = 0; j < numheadcols; j++) 
      fscanf(input, "%s", strng); // read in and disregard header columns

    for (int j = 0; j < numInCols; j++) {
      if(feof(input))
	fatal("Input file is missing data");

      fscanf(input, "%s", strng);
      //cout << i << ", " << j << ": " << strng << endl;

      ascii1 = strng[0]; // find ascii value of first char

      if (space) {
	fscanf(input, "%s", strng); // read in second allele
	ascii2 = strng[0]; // find ascii value of first char
	
      }
      
      if (!space) { // no space, so second allele is in current string
	if (slash) 
	  ascii2 = strng[2]; // second allele is third char, after '/'
	else
	  ascii2 = strng[1]; // second allele is second char

	if ((ascii1 == 48) || (ascii1 == 78)) // missing data
	  ascii2 = 78; // set to missing as might have 'NA' in input

	if(ascii1 == MISSING_SYMBOL) // customized missing symbol
	  ascii2 = 78; // set to missing 
      }
      
      // check validity of data
      if ((ascii1 != 65) && (ascii1 != 67)) // not 'A' or 'C'
	if ((ascii1 != 71) && (ascii1 != 84))  // not 'G' or 'T' 
	  if ((ascii1 != 73) && (ascii1 != 68))  // not 'I' or 'D'
	    if ((ascii1 != 48) && (ascii1 != 78))  // not 'N' or '0'
	      if(ascii1 != MISSING_SYMBOL) // not customized missing symbol
	      if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
		cout << i << ", " << j << ": " << endl;
		cout << (char)ascii1 << endl;
		fatal("Improper input data");
	      }
      
      if ((ascii2 != 65) && (ascii2 != 67)) // not 'A' or 'C'
	if ((ascii2 != 71) && (ascii2 != 84))  // not 'G' or 'T' 
	  if ((ascii2 != 73) && (ascii2 != 68))  // not 'I' or 'D'
	    if ((ascii2 != 48) && (ascii2 != 78)) // not 'N' or '0'
	      if(ascii2 != MISSING_SYMBOL) // not customized missing symbol
	      if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
		cout << i << ", " << j << ": " << endl;
		cout << (char)ascii2 << endl;
		fatal("Improper input data");
	      }
      
      // determine alleles for each genotype

      int currentSNP; // get current SNP number
      if (ROWS_R_SNPS == 0)
	currentSNP = j; // current SNP is current column number
      if (ROWS_R_SNPS == 1)
	currentSNP = i; // current SNP is current row number

      if ((ascii1 != 48) && (ascii1 != 78))  // not 'N' or '0'
	if(ascii1 != MISSING_SYMBOL) // not customized missing symbol
	  if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
	    if(allele[currentSNP][0] == '0') // '0' so haven't found first allele yet
	      allele[currentSNP][0] = (char)ascii1;
	    else if (allele[currentSNP][1] == '0') // '0' so haven't found second allele yet
	      if (ascii1 != (int)allele[currentSNP][0])
		allele[currentSNP][1] = (char)ascii1; // found second allele
	    
	    if (allele[currentSNP][1] == '0') // '0' so haven't found second allele yet
	      if (ascii2 != (int)allele[currentSNP][0])
		allele[currentSNP][1] = (char)ascii2; // found second allele
	    
	    //cout << (char)ascii1 << (char)ascii2 << " "; 
	    //cout << allele[currentSNP][0] << allele[currentSNP][1] << endl;
	  }
    }
  }
  
  // order alleles alphabetically
  for (int i = 0; i < numSnps; i++) 
    if (allele[i][0] > allele[i][1]) { // exchange values
      int temp = allele[i][0];
      allele[i][0] = allele[i][1];
      allele[i][1] = temp;
    }

  // check for only one allele for a SNP
  int oneAllele = 0; // number of SNPs with only one allele

  for (int i = 0; i < numSnps; i++)
    if (allele[i][0] == '0')
      oneAllele++;

  cout << oneAllele << " snps have only one allele in dataset." << endl;

  if(LOG_FILE)
    fprintf(logfile, "%d snps have only one allele in dataset.\n", oneAllele);

  if (VERBOSE) {
    cout << "Alleles: " << endl;
    for (int i = 0; i < numSnps; i++) {
      for (int j = 0; j < 2; j++)
	cout << allele[i][j] << " ";
      cout << endl;
    }
  }

  // check for end of file
  if (!feof(input)){
    fscanf(input, "%s", strng);
    if (!feof(input))
      fatal("Unread data in input file");
  }

  // allocate space for tallies of individuals without missing genotypes
  int *haveGenotype1; // first set of SNPs
  int *haveGenotype2; // second set of SNPs

  if ((haveGenotype1 = new int[numSnps1]) == NULL)
    fatal("memory not allocated");

  if ((haveGenotype2 = new int[numSnps2]) == NULL)
    fatal("memory not allocated");

  // initialize values to number of individuals and subtract missing
  for (int i = 0; i < numSnps1; i++)
    haveGenotype1[i] = numInd;

  for (int i = 0; i < numSnps2; i++)
    haveGenotype2[i] = numInd;

  // reread data again and record values
  rewind(input); // rewind to beginning of file

  // read in header rows and disregard
  for (int i = 0; i < numheadrows; i++)
    for (int j = 0; j < numheadcols + numInCols; j++)
      fscanf(input, "%s", strng); 

  // read in data
  for (int i = 0; i < numInRows; i++) {
    for (int j = 0; j < numheadcols; j++) 
      fscanf(input, "%s", strng); // read in and disregard header columns

    for (int j = 0; j < numInCols; j++) {
      fscanf(input, "%s", strng);
      //cout << i << ", " << j << ": " << strng << endl;

      ascii1 = strng[0]; // find ascii value of first char

      if (space) {
	fscanf(input, "%s", strng); // read in second allele
	ascii2 = strng[0]; // find ascii value of first char
	
      }
      
      if (!space) { // no space, so second allele is in current string
	if (slash) 
	  ascii2 = strng[2]; // second allele is third char, after '/'
	else
	  ascii2 = strng[1]; // second allele is second char

	if ((ascii1 == 48) || (ascii1 == 78)) // missing data
	  ascii2 = 78; // set to missing as might have 'NA' in input

	if(ascii1 == MISSING_SYMBOL) // customized missing symbol
	  ascii2 = 78; // set to missing
      }
      
      // assign index depending upon whether SNPs are represented by rows or columns
    
      int currentSNP; // get current SNP number
      int currentInd; // get current individual number
    
      if (ROWS_R_SNPS == 0) {
	currentSNP = j; // current SNP is current column number
	currentInd = i; // current individual is row number
      }

      if (ROWS_R_SNPS == 1) {
	currentSNP = i; // current SNP is current row number
	currentInd = j; // current individual is column number
      }

      // check validity of input data
      
      if ((ascii1 != (int)allele[currentSNP][0]) && (ascii1 != (int)allele[currentSNP][1])) // not one of the alleles 
	  if ((ascii1 != 48) && (ascii1 != 78)) // not 'N' or '0'
	    if(ascii1 != MISSING_SYMBOL) // not customized missing symbol
	    if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
	      cout << "SNP " << currentSNP+1 << " for individual " << currentInd+1 << " (" << allele[currentSNP][0] << allele[currentSNP][1] << ")" << endl;
	      cout << "Doesn't match: " << (char)ascii1 << endl;
	      fatal("***Invalid data treated as missing.  Are there more than 2 alleles?***");
	      ascii1 = 48; // treat as missing data
	    }
      
      if ((ascii2 != (int)allele[currentSNP][0]) && (ascii2 != (int)allele[currentSNP][1])) // not one of the alleles 
	if ((ascii2 != 48) && (ascii2 != 78)) // not 'N' or '0'
	  if(ascii2 != MISSING_SYMBOL) // not customized missing symbol
	  if ((ascii2 != 63) && (ascii2 != 88)) { // not '?' or 'X'
	    cout << "SNP " << currentSNP+1 << " for individual " << currentInd+1 << " (" << allele[currentSNP][0] << allele[currentSNP][1] << ")" << endl;
	    cout << "Doesn't match: " << (char)ascii2 << endl;
	    fatal("***Invalid data treated as missing.  Are there more than 2 alleles?***");
	    ascii2 = 48; // treat as missing data
	  }
      
      // assign codes for genotypes
      // record if in first set of SNPs
      if((currentSNP >= start1) && (currentSNP <= end1)) { // record value
	int k = currentSNP - start1; // index in data matrix
	
	if ((k < 0) || (k >= numSnps1))
	  fatal("Error in data matrix");
	
	if ((ascii1 == 48) || (ascii1 == 78)) { // 'N' or '0', mark as 3
	  data1[k][currentInd] = 3; // table with numSnps rows
	  haveGenotype1[k]--; // one less individual with genotype given
	  totalNumMissing1++; // count total number missing
	}
	
	else
	  if(ascii1 == MISSING_SYMBOL) { // customized missing symbol
	    data1[k][currentInd] = 3; // table with numSnps rows
	    haveGenotype1[k]--; // one less individual with genotype given
	    totalNumMissing1++; // count total number missing
	  }

	  else
	    if ((ascii1 == 63) || (ascii1 == 88)) { // '?' or 'X', mark as 3
	      data1[k][currentInd] = 3; // table with numSnps rows
	      haveGenotype1[k]--; // one less individual with genotype given
	      totalNumMissing1++; // count total number missing
	    }
	
	    else
	      if (ascii1 != ascii2) { // heterozygous
		data1[k][currentInd] = 1;
		freq1[k][0]++; // another allele in data
		freq1[k][1]++; // another allele in data
	      } 
	
	      else
		if (ascii1 == (int)allele[currentSNP][0]) { // homozygote in first allele
		  data1[k][currentInd] = 0;
		  freq1[k][0]+= 2; // two more alleles in data
		}
	
		else
		  if (ascii1 == (int)allele[currentSNP][1]) { // homozygote in second allele
		    data1[k][currentInd] = 2;
		    freq1[k][1]+= 2; // two more alleles in data
		  }
      }

      //cout << currentSNP << ", start2 = " << start2 << ", end2 = " << end2 << endl;
    
      // record if in second set of SNPs
      if((currentSNP >= start2) && (currentSNP <= end2)) { // record value
	int k = currentSNP - start2; // index in data matrix
	
	if ((k < 0) || (k >= numSnps2))
	  fatal("Error in data matrix");
	
	if ((ascii1 == 48) || (ascii1 == 78)) { // 'N' or '0', mark as 3
	  data2[k][currentInd] = 3; // table with numSnps rows
	  haveGenotype2[k]--; // one less individual with genotype given
	  totalNumMissing2++; // count total number missing
	  continue;
	}
	
	if(ascii1 == MISSING_SYMBOL) { // customized missing symbol
	  data2[k][currentInd] = 3; // table with numSnps rows
	  haveGenotype2[k]--; // one less individual with genotype given
	  totalNumMissing2++; // count total number missing
	  continue;
	}
	
	if ((ascii1 == 63) || (ascii1 == 88)) { // '?' or 'X', mark as 3
	  data2[k][currentInd] = 3; // table with numSnps rows
	  haveGenotype2[k]--; // one less individual with genotype given
	  totalNumMissing2++; // count total number missing
	  continue;
	}
	
	if (ascii1 != ascii2) { // heterozygous
	  data2[k][currentInd] = 1;
	  freq2[k][0]++; // another allele in data
	  freq2[k][1]++; // another allele in data
	  continue;
	} 
	
	if (ascii1 == (int)allele[currentSNP][0]) { // homozygote in first allele
	  data2[k][currentInd] = 0;
	  freq2[k][0]+= 2; // two more alleles in data
	  continue;
	}
	
	if (ascii1 == (int)allele[currentSNP][1]) { // homozygote in second allele
	  data2[k][currentInd] = 2;
	  freq2[k][1]+= 2; // two more alleles in data
	  continue;
	}
      }
    }
  }
      
  // check for end of file
  if (!feof(input)){
    fscanf(input, "%s", strng);
    if (!feof(input))
      fatal("Unread data in input file");
  }

  fclose(input);

  // convert frequency counts to frequency factors

  // print out frequencies, if parameter set
  FILE *tempFreq;

  if(printFreq)
    if ((tempFreq = fopen("temp.freq", "w")) == NULL)
      fatal("Frequency file could not be opened.\n");
  

  if(VERBOSE) 
    cout << "Frequencies of alleles in first set:" << endl;

  for (int i = 0; i < numSnps1; i++) {

    if (printFreq)
      fprintf(tempFreq, "%d", i+1);
 
    for (int j = 0; j < 2; j++) {
      freq1[i][j] /= 2 * haveGenotype1[i]; // divide by 2*number without missing

      if(VERBOSE) 
	cout << freq1[i][j] << endl;

      if (printFreq)
	fprintf(tempFreq, " %c %f", allele[i][j], freq1[i][j]);
     
      // calculate frequency factor
      freq1[i][j] = 1 - (freq1[i][j] / FREQWT); 

      if(0) 
	cout << freq1[i][j] << endl;
    }

    if (printFreq)
      fprintf(tempFreq, "\n");
  }

  if(VERBOSE) 
    cout << "Frequencies of alleles in second set:" << endl;

  for (int i = 0; i < numSnps2; i++) {

    //if (printFreq)  // removed as only print out first set of frequencies
    //fprintf(tempFreq, "%d", i+1);
 
    for (int j = 0; j < 2; j++) {
      freq2[i][j] /= 2 * haveGenotype2[i]; // divide by 2*number without missing

      if(VERBOSE) 
	cout << freq2[i][j] << endl;

      //if (printFreq)
      //fprintf(tempFreq, " %c %f", allele[i][j], freq[i][j]);
     
      // calculate frequency factor
      freq2[i][j] = 1 - (freq2[i][j] / FREQWT); 

      if(0) 
	cout << freq2[i][j] << endl;
    }

    //if (printFreq)
    //fprintf(tempFreq, "\n");
  }

  if (printFreq)
    fclose(tempFreq);

  if (VERBOSE) {
    cout << "Encoded data for start SNPs (number of alleles with highest alphabetic order):"<< endl;
    for (int i = 0; i < numSnps1; i++) {
      for (int j = 0; j < numInd; j++)
	cout << (int)data1[i][j] << " ";
      cout << endl;
    }

    cout << "Encoded data for end SNPs (number of alleles with highest alphabetic order):"<< endl;
    for (int i = 0; i < numSnps2; i++) {
      for (int j = 0; j < numInd; j++)
	cout << (int)data2[i][j] << " ";
      cout << endl;
    }
  }
  
  // check validity of data
  for (int i = 0; i < numSnps1; i++) 
    for (int j = 0; j < numInd; j++)
      if (data1[i][j] > 3) {
	cout << "Data1 " << i << ", " << j << ": " << (int)data1[i][j] << endl;
	fatal("Invalid value in data matrix");
      }

  for (int i = 0; i < numSnps2; i++) 
    for (int j = 0; j < numInd; j++)
      if (data2[i][j] > 3) {
	cout << "Data2 " << i << ", " << j << ": " << (int)data2[i][j] << endl;
	fatal("Invalid value in data matrix");
      }
  
  delete [] haveGenotype1;
  delete [] haveGenotype2;

  cout << totalNumMissing1 << " and " << totalNumMissing2 << " missing values in first and second SNP sets, respectively." << endl;

  if(LOG_FILE)
    fprintf(logfile, "%d and %d missing values in first and second SNP sets, respectively.\n", totalNumMissing1, totalNumMissing2); 

  fclose(logfile);

}


void checkConstants()
{
  // check Boolean values 

  if(SCREEN_INPUT == 1)
    fatal("Need to fix code for SCREEN_INPUT set to 1 in bloc.h.");

  if((SCREEN_INPUT != 0) && (SCREEN_INPUT != 1))
    fatal("SCREEN_INPUT value in bloc.h should be zero or one.");

  if((LOG_FILE != 0) && (LOG_FILE != 1))
    fatal("LOG_FILE value in bloc.h should be zero or one.");

  if((QUIET != 0) && (QUIET != 1))
    fatal("QUIET value in bloc.h should be zero or one.");

  if((VERBOSE != 0) && (VERBOSE != 1))
    fatal("VERBOSE value in bloc.h should be zero or one.");

  if((PRINTGML != 0) && (PRINTGML != 1))
    fatal("PRINTGML value in bloc.h should be zero or one.");

  if((TWONODE != 0) && (TWONODE != 1))
    fatal("TWONODE value in bloc.h should be zero or one.");

  if((PRINTFREQ != 0) && (PRINTFREQ != 1))
    fatal("PRINTFREQ value in bloc.h should be zero or one.");

  if((FREQ != 0) && (FREQ != 1))
    fatal("FREQ value in bloc.h should be zero or one.");

  // check other values
  if ((FREQWT > 1.5 + TOL) || (FREQWT < 1.5 - TOL))
    warning("Default frequency weight is 1.5.  Check FREQWT in bloc.h");
 
  if((NOMISS > 1.0 + TOL) || (NOMISS < 0.0 - TOL))
    warning("Invalid value for NOMISS in bloc.h.");
 
  if((TOL > 0.001) || (TOL < -0.0000001))
    fatal("Invalid value for TOL in bloc.h.");
}
