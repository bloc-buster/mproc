/*   
 *
 *   carriers.cpp 
 *
 *   Program to identify carriers of combinatorial SNP allele patterns.
 *   Genotypes of Cases and Controls are used to compare the clusters
 *   Clusters are defined by a file with BFS format
 *
 *   Written by Sharlee Climer
 *   June 2009
 *
 *   Simplified by Sharlee Climer
 *   June 2014
 *
 */


#include "carriers.h"

using namespace std;

int main(int argc, char ** argv)
{
  if (argc != 13)
    fatal("Usage:\n  carriers clusters.bfs cases.genotypes controls.genotypes numHeadRowsGen numHeadColsGen snp_info.txt numColsSNPinfo numHeadRowsSNPinfo numCases numControls numSNPs outputFile.txt"); 

  timer t;

  t.start("Timer started");  

  FILE *bfs; // contains clusters
  FILE *cases; // contains Cases genotypes
  FILE *ctrl; // contains Controls genotypes
  FILE *info; // contains SNP annotation information
  FILE *output; // will hold annotations of significant clusters

  cout << "Command line arguments: \n\t";
  for (int i = 0; i < argc; i++)
	cout << argv[i] << " ";
  cout << "\n" << endl;

  // check to be sure all files are available
  if ((bfs = fopen(argv[1], "r")) == NULL)
    fatal("cluster file could not be opened");
  if (((cases = fopen(argv[2], "r")) == NULL) || ((ctrl = fopen(argv[3], "r")) == NULL))
    fatal("Input file could not be opened.\n");
  if ((info = fopen(argv[6], "r")) == NULL)
	fatal("Info file could not be opened");

  fclose(cases);
  fclose(ctrl);
  fclose(info);

  int numHeadRowsGen = atoi(argv[4]); // number of header rows in genotype data files
  int numHeadColsGen = atoi(argv[5]); // number of header columns in genotype data files

  int numColsInfo = atoi(argv[7]); // number of columns in SNP info files
  int numHeadRowsInfo = atoi(argv[8]); // number of header rows in SNP info files

  int nCase = atoi(argv[9]);  // number of Cases individuals
  int nCtrl = atoi(argv[10]); // number of Controls
  int numSnps = atoi(argv[11]);  // number of SNPs
  int numAlleles = 2 *  numSnps; // number of alleles (assumed biallelic)

  cout << "Checking hypothetical clusters in '" << argv[1] << "' for '";
  cout << argv[2] << "' and '" << argv[3] << "'." << endl;

  cout << nCase << " Cases and " << nCtrl << " Controls." << endl;
  cout << numSnps << " SNPs.\n" << endl;

  //cout << "Assumed first " << NUMHEADCOLS << " columns do not contain genotype data.\n" << endl;

  cout << "\n  *** IMPORTANT: Assumed each row of genotype input data corresponds to an individual.***\n" << endl;

  if ((nCase > MAX_NUM_INDIVIDUALS) || (nCtrl > MAX_NUM_INDIVIDUALS))
	fatal("Too many individuals.  Fix header file.");
  if (numSnps > MAX_NUM_SNPs)
	fatal("Too many SNPs.  Fix header file.");
  if ((nCase < 1) || (nCtrl < 1))
	fatal("Invalid number of individuals");
  if (numSnps < 1)
	fatal("Invalid number of SNPs");

  int N = (int)'0';  // symbol used for missing data
  int numNN = 0;  // number of missing genotype values
  char strng[200];

  // read in cluster information
  fscanf(bfs, "%s", strng); // read in number of nodes
  int num = atoi(strng);
  if (num != numAlleles)
    fatal("Number of nodes in cluster file not equal to number of alleles");

  for (int i = 0; i < 5; i++) {
    fscanf(bfs,"%s",strng); // read in and throw away header information
    //cout << strng << endl;
  }

  if (!strncmp(strng, "edges", 5) == 0) // check text
    fatal("Expected the word edges at end of header of cluster file");


  // find the number of clusters and count singletons
  int numClusters = -1; // number of clusters
  int numSingle = 0; // number of singletons
  int startZero = 0; // set to 1 if start cluster numbers start at 0

  for (int i = 0; i < numAlleles; i++) {
    fscanf(bfs,"%s",strng); // read in cluster numbers
    num = atoi(strng);
    if (num < -1)
      fatal("cluster number is less than -1");
    if (num == -1)
      numSingle++; // a singleton

    if(num == 0) {
       startZero = 1; // cluster numbering starts at zero
       if (!strncmp(strng, "0", 1) == 0)
	 fatal("Invalid value in cluster file");
    }

    if (num > numSnps) // can't have more clusters than there are SNPs
      fatal("invalid cluster number in input file");

    if (num > numClusters)
      numClusters = num; // find largest cluster number
  }

  if (startZero)
    numClusters++; // cluster numbers start with a zero

  cout << numClusters << " clusters to be tested." << endl;

  fclose(bfs); // close for now, reopen when ready to read in clusters

  // allocate memory for cluster information
  int **clusts; // hold alleles for each cluster
  int *clustSize; // hold number of alleles for each cluster

  if ((clusts = new int* [numClusters]) == NULL)
    fatal("Memory not allocated");

  // max number of alleles in a cluster is limited to MAX_SIZE 
  for (int i = 0; i < numClusters; i++)
    if ((clusts[i] = new int[MAX_SIZE]) == NULL) // limit size of cluster tested
      fatal("Memory not allocated");

  if ((clustSize = new int[numClusters]) == NULL)
    fatal("Memory not allocated");

  for (int i = 0; i < numClusters; i++)
    for (int j = 0; j < MAX_SIZE; j++)
      clusts[i][j] = -1; // initialize clusts matrix

  for (int i = 0; i < numClusters; i++)
    clustSize[i] = 0; // initialize cluster sizes

  // read in cluster information
  if ((bfs = fopen(argv[1], "r")) == NULL)
    fatal("cluster file could not be opened");

  for (int i = 0; i < 6; i++)
    fscanf(bfs,"%s",strng); // read in and throw away header information
  if (!strncmp(strng, "edges", 5) == 0) // check text
    fatal("Expected the word edges at end of header of cluster file");

  for (int i = 0; i < numAlleles; i++) {
    fscanf(bfs,"%s",strng); // read in cluster numbers
    num = atoi(strng);
    if (num > -1) { // not a singleton
	  if(clustSize[num] == MAX_SIZE) // already have maximum number of nodes
		continue; // ignore additional nodes

      clusts[num][clustSize[num]] = i; // allele i is in this cluster
      clustSize[num]++; // increase the number of alleles in this cluster
    }
  }

  int maxClustSize = 0; // record maximum cluster size
  int numMax = 0; // record number with maximum size

  for (int i = 0; i < numClusters; i++) {
    if (clustSize[i] == 0)
      warning("cluster with no nodes in cluster file");
    if (clustSize[i] == 1)
      numSingle++; // a singleton that wasn't marked '-1'
    if(clustSize[i] > maxClustSize)
      maxClustSize = clustSize[i];
	if(clustSize[i] == MAX_SIZE)
	  numMax++;
  }

  if(numMax > 0) {
	cout << numMax << " clusters have more than " << MAX_SIZE << " nodes." << endl;
	cout << "Only the first " << MAX_SIZE << " were tested for these clusters.\n" << endl;
  }

  cout << numSingle << " singletons in cluster file." << endl;
  cout << "Largest cluster has " << maxClustSize << " nodes.\n" << endl;
  if (VERBOSE) {
    cout << "Clusters: " << endl;
    for (int i = 0; i < numClusters; i++) {
      for (int j = 0; j < clustSize[i]; j++)
	cout << clusts[i][j]+1 << " ";
      cout << endl;
    }
  }

  // allocate memory
  int *numCase; // hold number of Cases with the cluster
  int *numCtrl;  // hold number of Controls with the cluster
  int *alleles; // hold the alleles for the current individual
  int **allelePairs; // hold ASCII values of alleles for each SNP
  int *numCase_noMissData; // hold number of cases that have no missing data for the SNPs in the cluster
  int *numCtrl_noMissData; // hold number of controls that have no missing data for the SNPs in the cluster

  if ((numCase = new int [numClusters]) == NULL)
    fatal("Memory not allocated");

  if ((numCtrl = new int [numClusters]) == NULL)
    fatal("Memory not allocated");

  if ((alleles = new int [numAlleles]) == NULL)
    fatal("Memory not allocated");

  if ((allelePairs = new int*[numSnps]) == NULL)
    fatal("Memory not allocated");

  for (int i = 0; i < numSnps; i++) // for each SNP, record two alleles
    if ((allelePairs[i] = new int [2]) == NULL)
      fatal("Memory not allocated");

  if ((numCase_noMissData = new int [numClusters]) == NULL)
    fatal("Memory not allocated");

  if ((numCtrl_noMissData = new int [numClusters]) == NULL)
    fatal("Memory not allocated");

  for (int i = 0; i < numClusters; i++) { // initialize arrays
    numCase[i] = 0;
    numCtrl[i] = 0;
    numCase_noMissData[i] = 0;
    numCtrl_noMissData[i] = 0;
  }
  
  for (int i = 0; i < numSnps; i++)  // initialize allelePairs
    for (int j  = 0; j < 2; j++)
      allelePairs[i][j] = -1;
  
  // determine format and the allele pair for each SNP, look at Controls file first
  if ((ctrl = fopen(argv[3], "r")) == NULL)
    fatal("Input file could not be opened.\n");

  int space = 0; // set to 1 if space between alleles in ctrl
  int slash = 0; // set to 1 if slash mark between alleles in ctrl
  int format = 0; // set to 1 once format is determined

  int ascii1, ascii2; // temporary values to hold alleles as they are read

  // read in header rows and disregard
  for (int i = 0; i < numHeadRowsGen; i++)
    for (int j = 0; j < numHeadColsGen + numSnps; j++)
      fscanf(ctrl, "%s", strng); 

  // read in data
  for (int i = 0; i < nCtrl; i++) {
    if (format)
      break; // already determined format

    for (int j = 0; j < numHeadColsGen; j++) 
      fscanf(ctrl, "%s", strng); // read in and disregard header columns

    for (int j = 0; j < numSnps; j++) {
      if(feof(ctrl))
	fatal("ctrl file is missing data");

      fscanf(ctrl, "%s", strng);
      //cout << i << ", " << j << ": " << strng << endl;

      num = strng[0]; // find ascii value of first char

      if ((num != 48) && (num != 78)) // not 'N' or '0'
	if ((num != 63) && (num != 88)) { // not '?' or 'X'
	
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

  rewind(ctrl); // start over at beginning of file

  // read in header rows and disregard
  for (int i = 0; i < numHeadRowsGen; i++)
    for (int j = 0; j < numHeadColsGen + numSnps; j++)
      fscanf(ctrl, "%s", strng); 
  
  // read in data for alleles Ctrl
  for (int i = 0; i < nCtrl; i++) {
    for (int j = 0; j < numHeadColsGen; j++) 
      fscanf(ctrl, "%s", strng); // read in and disregard header columns

    for (int j = 0; j < numSnps; j++) {
      if(feof(ctrl))
	fatal("Ctrl file is missing data");

      fscanf(ctrl, "%s", strng);
      //cout << i << ", " << j << ": " << strng << endl;

      ascii1 = strng[0]; // find ascii value of first char

      if (space) {
	fscanf(ctrl, "%s", strng); // read in second allele
	ascii2 = strng[0]; // find ascii value of first char
	
      }
      
      if (!space) { // no space, so second allele is in current string
	if (slash) 
	  ascii2 = strng[2]; // second allele is third char, after '/'
	else
	  ascii2 = strng[1]; // second allele is second char

	if ((ascii1 == 48) || (ascii1 == 78)) // missing data
	  ascii2 = 78; // set to missing as might have 'NA' in input
      }
      
      // check validity of data
      if ((ascii1 != 65) && (ascii1 != 67)) // not 'A' or 'C'
	if ((ascii1 != 71) && (ascii1 != 84))  // not 'G' or 'T' 
	  if ((ascii1 != 73) && (ascii1 != 68))  // not 'I' or 'D'
	    if ((ascii1 != 48) && (ascii1 != 78))  // not 'N' or '0'
	      if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
		cout << (char)ascii1 << endl;
		fatal("Improper input data in control file");
	      }
      
      if ((ascii2 != 65) && (ascii2 != 67)) // not 'A' or 'C'
	if ((ascii2 != 71) && (ascii2 != 84))  // not 'G' or 'T' 
	  if ((ascii2 != 73) && (ascii2 != 68))  // not 'I' or 'D'
	    if ((ascii2 != 48) && (ascii2 != 78)) // not 'N' or '0'
	      if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
		cout << (char)ascii2 << endl;
		fatal("Improper input data in control file");
	      }
      
      // determine alleles for each genotype
      if ((ascii1 != 48) && (ascii1 != 78))  // not 'N' or '0'
	if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
	  if(allelePairs[j][0] == -1) // '0' so haven't found first allele yet
	    allelePairs[j][0] = (char)ascii1;
	  else if (allelePairs[j][1] == -1) // '0' so haven't found second allele yet
	    if (ascii1 != (int)allelePairs[j][0])
	      allelePairs[j][1] = (char)ascii1; // found second allele
	  
	  if (allelePairs[j][1] == -1) // '0' so haven't found second allele yet
	    if (ascii2 != (int)allelePairs[j][0])
	      allelePairs[j][1] = (char)ascii2; // found second allele
	  
	  //cout << (char)ascii1 << (char)ascii2 << " "; 
	  //cout << allele[j][0] << allele[j][1] << endl;
	}
    }
  }
  
  fclose(ctrl); // close for now, will reopen to read genotypes

  if (((cases = fopen(argv[2], "r")) == NULL))
    fatal("Input file could not be opened.\n");

   // read in header rows and disregard
  for (int i = 0; i < numHeadRowsGen; i++)
    for (int j = 0; j < numHeadColsGen + numSnps; j++)
      fscanf(cases, "%s", strng); 

  // read in data for alleles cases
  for (int i = 0; i < nCase; i++) {
    for (int j = 0; j < numHeadColsGen; j++) 
      fscanf(cases, "%s", strng); // read in and disregard header columns

    for (int j = 0; j < numSnps; j++) {
      if(feof(cases))
	fatal("Case file is missing data");

      fscanf(cases, "%s", strng);
      //cout << i << ", " << j << ": " << strng << endl;

      ascii1 = strng[0]; // find ascii value of first char

      if (space) {
	fscanf(cases, "%s", strng); // read in second allele
	ascii2 = strng[0]; // find ascii value of first char
	
      }
      
      if (!space) { // no space, so second allele is in current string
	if (slash) 
	  ascii2 = strng[2]; // second allele is third char, after '/'
	else
	  ascii2 = strng[1]; // second allele is second char

	if ((ascii1 == 48) || (ascii1 == 78)) // missing data
	  ascii2 = 78; // set to missing as might have 'NA' in input
      }
      
      // check validity of data
      if ((ascii1 != 65) && (ascii1 != 67)) // not 'A' or 'C'
	if ((ascii1 != 71) && (ascii1 != 84))  // not 'G' or 'T' 
	  if ((ascii1 != 73) && (ascii1 != 68))  // not 'I' or 'D'
	    if ((ascii1 != 48) && (ascii1 != 78))  // not 'N' or '0'
	      if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
		cout << (char)ascii1 << endl;
		fatal("Improper input data in case file");
	      }
      
      if ((ascii2 != 65) && (ascii2 != 67)) // not 'A' or 'C'
	if ((ascii2 != 71) && (ascii2 != 84))  // not 'G' or 'T' 
	  if ((ascii2 != 73) && (ascii2 != 68))  // not 'I' or 'D'
	    if ((ascii2 != 48) && (ascii2 != 78)) // not 'N' or '0'
	      if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
		cout << (char)ascii2 << endl;
		fatal("Improper input data in case file");
	      }
      
      // determine alleles for each genotype
      if ((ascii1 != 48) && (ascii1 != 78))  // not 'N' or '0'
	if ((ascii1 != 63) && (ascii1 != 88)) { // not '?' or 'X'
	  if(allelePairs[j][0] == -1) // '0' so haven't found first allele yet
	    allelePairs[j][0] = (char)ascii1;
	  else if (allelePairs[j][1] == -1) // '0' so haven't found second allele yet
	    if (ascii1 != (int)allelePairs[j][0])
	      allelePairs[j][1] = (char)ascii1; // found second allele
	  
	  if (allelePairs[j][1] == -1) // '0' so haven't found second allele yet
	    if (ascii2 != (int)allelePairs[j][0])
	      allelePairs[j][1] = (char)ascii2; // found second allele
	  
	  //cout << (char)ascii1 << (char)ascii2 << " "; 
	  //cout << allele[j][0] << allele[j][1] << endl;
	}
    }
  }
  
  fclose(cases); // close for now, will reopen to read genotypes

  int numMono = 0; // number of mono-allelic SNPs in Controls
  
  // sort allele pairs alphabetically and check for legitimate values
  for (int i = 0; i < numSnps; i++) {
    for (int j = 0; j < 2; j++) {
      num = allelePairs[i][j];
	  //cout << num << endl;
      if (num == -1) {
		if (j == 0)
		  fatal("Invalid allele value");
		else {
		  numMono++; // a mono-allelic SNP
		  num = allelePairs[i][1] = allelePairs[i][0]; // both alleles the same
		}
      }
	  
      if ((num != 65) && (num != 67))  // not 'A' or 'C'
        if ((num != 73) && (num != 68)) // not 'D' or 'I'
		if ((num != 71) && (num != 84))  { // not 'G' or 'T'
		  cout << i << ", " << j << ": " << (char)allelePairs[i][j] << endl;
		  fatal("improper SNP input data");
		}
    }
	
    if (allelePairs[i][0] > allelePairs[i][1]) { // not in alphabetical order
      int temp = allelePairs[i][0];  // switch
      allelePairs[i][0] = allelePairs[i][1];
      allelePairs[i][1] = temp;
    }
  }
  
  if (VERBOSE) {
    cout << "Allele Pairs:" << endl;
    for (int i = 0; i < numSnps; i++)
      cout << (char)allelePairs[i][0] << " / " << (char)allelePairs[i][1]  << endl;
    cout << endl;
  }

  if (numMono > 0) {
    cout << numMono << " mono-allelic SNPs in Controls data" << endl;
  }

  int missing; // set to 1 if missing data
  int numMissing = 0; // count number of missing values
 
  // read in Cases genotypes and tally those who have the allele clusters
  if ((cases = fopen(argv[2], "r")) == NULL)
    fatal("Input file could not be opened.\n");

  // read in header rows and disregard
  for (int i = 0; i < numHeadRowsGen; i++)
    for (int j = 0; j < numHeadColsGen + numSnps; j++)
      fscanf(cases, "%s", strng);
  
  for (int i = 0; i < nCase; i++) { // read in each individual's gentypes
    for (int j = 0; j < numAlleles; j++) // initialize array
      alleles[j] = 0; // number of alleles for this individual
    
    for (int j = 0; j < numHeadColsGen; j++)
      fscanf(cases, "%s", strng); // read in header columns and throw away
    
    for (int j = 0; j < numSnps; j++) { // input has SNPs in the columns
      missing = 0; // intialize flag
      fscanf(cases, "%s", strng); // read in first allele for the genotype
      ascii1 = strng[0];
      //cout << num << " ";
      
      if ((ascii1 != 48) && (ascii1 != 78)) { // don't use 'N' or '0' -- missing data
	if(ascii1 == allelePairs[j][0])  // lowest alphabetically-ordered allele
	  alleles[j]++;  // individual has this allele
	else {
	  if (ascii1 != allelePairs[j][1]) {
	    cout << "(" << i << "," << j << ") " << (char)ascii1 << ": " ;
	    cout << (char)allelePairs[j][0] << ", " << (char)allelePairs[j][1] << endl; 
	    fatal("Invalid allele in Cases genotype file");
	  }
	  alleles[j + numSnps]++; // individual has an allele with highest alphabetic order
	}	  
      }
      
      else { // missing data
	missing = 1; // set missing flag
	alleles[j] = alleles[j+numSnps] = -1; // mark as missing
      }
      
      if (space) {
	fscanf(cases, "%s", strng); // read in second allele
	ascii2 = strng[0]; // find ascii value of first char
	
      }
      
      if (!space) { // no space, so second allele is in current string
	if (slash) 
	  ascii2 = strng[2]; // second allele is third char, after '/'
	else
	  ascii2 = strng[1]; // second allele is second char

	if ((ascii1 == 48) || (ascii1 == 78)) // missing data
	  ascii2 = 78; // set to missing as might have 'NA' in input
      }
      
      if ((ascii2 != 48) && (ascii2 != 78)) { // don't use 'N' or '0' -- missing data
	if(ascii2 == allelePairs[j][0])  // lowest alphabetically-ordered allele
	  alleles[j]++;  // individual has this allele
	else {
	  if (ascii2 != allelePairs[j][1]) {
	    cout << "(" << i << "," << j << ") " << (char)ascii2 << ": " ;
	    cout << (char)allelePairs[j][0] << ", " << (char)allelePairs[j][1] << endl; 
	    fatal("Invalid allele in Cases genotype file");
	  }
	  alleles[j + numSnps]++; // individual has an allele with highest alphabetic order
	}
      }
      
      else if (!missing)
	fatal("One allele missing and other one isn't");
    }
    
    if (0) {
      cout << "Alleles for case " << i+1 << endl;
      for (int j = 0; j < numAlleles; j++) 
	cout << alleles[j]  << " " ;
      cout << endl;
    }
    
    // check each cluster
    for (int j = 0; j < numClusters; j++) {
      int score = 0; // score for this cluster for this individual
      int flag = 1; // set to 0 if individual does not have this cluster
      int totalDiff = 0; // number that are different
      int numMissing = 0; // number that are different due to missing values
      
      for (int k = 0; k < clustSize[j]; k++)
	if (alleles[clusts[j][k]] > 0) { // individual has this allele
	  score += alleles[clusts[j][k]]; // add number of alleles for this cluster
	}
      
	else { // individual doesn't have this allele
	  flag = 0; // this individual doesn't have the allele

	  if (alleles[clusts[j][k]] == -1)
	    numMissing++; // miss is due to missing data
	}
      
      if (flag == 1)
	numCase[j]++; // another individual with this cluster

      // check if individual has missing data for this pattern
      int missFlag = 0; // set to one if missing some of the pattern data
      for (int k = 0; k < clustSize[j]; k++)
	if (alleles[clusts[j][k]] < 0)  // missing data
	  missFlag++; // increment number of missing alleles
      if (missFlag == 0) // none missing
	numCase_noMissData[j]++; // increment # of ind. w/ no missing data
    }
  }

  // check for end of file
  if (!feof(cases)){
    fscanf(cases, "%s", strng);
    if (!feof(cases))
      fatal("Unread data in input file");
  }
  
  fclose(cases);

// read in Controls genotypes and tally those who have the allele clusters
 if ((ctrl = fopen(argv[3], "r")) == NULL)
    fatal("Input file could not be opened.\n");

  // read in header rows and disregard
  for (int i = 0; i < numHeadRowsGen; i++)
    for (int j = 0; j < numHeadColsGen + numSnps; j++)
      fscanf(ctrl, "%s", strng);
  
  for (int i = 0; i < nCtrl; i++) { // read in each individual's gentypes
    for (int j = 0; j < numAlleles; j++) // initialize array
      alleles[j] = 0; // number of alleles for this individual
    
    for (int j = 0; j < numHeadColsGen; j++)
      fscanf(ctrl, "%s", strng); // read in first 6 columns and throw away
    
    for (int j = 0; j < numSnps; j++) { // input has SNPs in the columns
      missing = 0; // intialize flag
      fscanf(ctrl, "%s", strng); // read in first allele for the genotype
      ascii1 = strng[0];
      //cout << num << " ";
      
      if ((ascii1 != 48) && (ascii1 != 78)) { // don't use 'N' or '0' -- missing data
	if(ascii1 == allelePairs[j][0])  // lowest alphabetically-ordered allele
	  alleles[j]++;  // individual has this allele
	else {
	  if (ascii1 != allelePairs[j][1]) {
	    cout << "(" << i << "," << j << ") " << (char)ascii1 << ": " ;
	    cout << (char)allelePairs[j][0] << ", " << (char)allelePairs[j][1] << endl; 
	    fatal("Invalid allele in ctrl genotype file");
	  }
	  alleles[j + numSnps]++; // individual has an allele with highest alphabetic order
	}	  
      }
      
      else { // missing data
	missing = 1; // set missing flag
	alleles[j] = alleles[j+numSnps] = -1; // mark as missing
      }
      
      if (space) {
	fscanf(ctrl, "%s", strng); // read in second allele
	ascii2 = strng[0]; // find ascii value of first char
	
      }
      
      if (!space) { // no space, so second allele is in current string
	if (slash) 
	  ascii2 = strng[2]; // second allele is third char, after '/'
	else
	  ascii2 = strng[1]; // second allele is second char

	if ((ascii1 == 48) || (ascii1 == 78)) // missing data
	  ascii2 = 78; // set to missing as might have 'NA' in input
      }
      
      if ((ascii2 != 48) && (ascii2 != 78)) { // don't use 'N' or '0' -- missing data
	if(ascii2 == allelePairs[j][0])  // lowest alphabetically-ordered allele
	  alleles[j]++;  // individual has this allele
	else {
	  if (ascii2 != allelePairs[j][1]) {
	    cout << "(" << i << "," << j << ") " << (char)ascii2 << ": " ;
	    cout << (char)allelePairs[j][0] << ", " << (char)allelePairs[j][1] << endl; 
	    fatal("Invalid allele in ctrl genotype file");
	  }
	  alleles[j + numSnps]++; // individual has an allele with highest alphabetic order
	}
      }

      else if (!missing)
	fatal("One allele missing and other one isn't");
    }
	
    if (0) {
      cout << "Alleles for control " << i+1 << endl;
      for (int j = 0; j < numAlleles; j++) 
		cout << alleles[j]  << " " ;
      cout << endl;
    }
	
    // check each cluster
    for (int j = 0; j < numClusters; j++) {
      int score = 0; // score for this cluster for this individual
      int flag = 1; // set to 0 if individual does not have this cluster
      int totalDiff = 0; // number that are different
      int numMissing = 0; // number that are different due to missing values
      
      for (int k = 0; k < clustSize[j]; k++)
	if (alleles[clusts[j][k]] > 0) { // individual has this allele
	  score += alleles[clusts[j][k]]; // add number of alleles for this cluster
	}
      
	else { // individual doesn't have this allele
	  flag = 0; // this individual doesn't have the allele
	  
	  if (alleles[clusts[j][k]] == -1)
	    numMissing++; // miss is due to missing data
	}
      
      if (flag == 1) 
	numCtrl[j]++; // another individual with this cluster
   
      // check if individual has missing data for this pattern
      int missFlag = 0; // set to one if missing some of the pattern data
      for (int k = 0; k < clustSize[j]; k++)
	if (alleles[clusts[j][k]] < 0)  // missing data
	  missFlag++; // increment number of missing alleles
      if (missFlag == 0) // none missing
	numCtrl_noMissData[j]++; // increment # of ind. w/ no missing data
    }
  }
  
  // check for end of file
  if (!feof(ctrl)){
    fscanf(ctrl, "%s", strng);
    if (!feof(ctrl))
      fatal("Unread data in input file");
  }

  fclose(ctrl);

  // output clusters

  float max; // highest percentage of individuals (Cases vs. Controls) with clusters
  float min; // lowest percentage " "
  float maxDiff = 0; // maximum difference found

  if ((info = fopen(argv[6], "r")) == NULL)
	fatal("Info file could not be opened");

  if ((output = fopen(argv[12], "w")) == NULL)
	fatal("Output file could not be opened");

  // read in SNP information
  char ***ann; // hold annotation information

  if ((ann = new char** [numSnps]) == NULL)
    fatal("Memory not allocated");

  for (int i = 0; i < numSnps; i++) {
    if ((ann[i] = new char*[numColsInfo]) == NULL)
      fatal("Memory not allocated");
	for (int j = 0; j < numColsInfo; j++)
	  if ((ann[i][j] = new char[200]) == NULL)
		fatal("Memory not allocated");
  }

  for (int row = 0; row < numHeadRowsInfo; row++) {
    fprintf(output, "Number\tAllele");
    for (int i = 0; i < numColsInfo; i++) {
      fscanf(info, "%s", strng);  // read in property labels
      fprintf(output, "\t%s", strng);  // write labels to annotation table
    }
  
    fprintf(output, "\n");
  }

  for (int i = 0; i < numSnps; i++) 
    for (int j = 0; j < numColsInfo; j++) {
      if(feof(info))
	fatal("Missing values in SNP info file");
      
      fscanf(info, "%s", ann[i][j]);
    }
  
  // check for end of file
  if(!feof(info)) {
	fscanf(info, "%s", strng);
	if (!feof(info))
	  fatal("Unread information in SNP data file");
  }

  fclose(info);
  
  if (VERBOSE) {
    for (int i = 0; i < numSnps; i++) {
      for (int j = 0; j < numColsInfo; j++) 
	cout << ann[i][j] << "\t";
      cout << endl;
    }
  }

  cout << endl;

  if (VERBOSE) {
    for (int i = 0; i < numClusters; i++)
      cout << "Cluster " << i+1 << " has " << clustSize[i] << " alleles and there are " << numCase_noMissData[i] << " cases and " << numCtrl_noMissData[i] << " controls with no missing data." << endl;
    cout << endl;
  }
  
  for (int i = 0; i < numClusters; i++) {
    
    float percentCase = 100.0*((float)numCase[i]/(float)numCase_noMissData[i]);
    float percentCtrl = 100.0*((float)numCtrl[i]/(float)numCtrl_noMissData[i]);

    if ((percentCase > TOL) || (percentCtrl > TOL)) {
      
      if (!QUIET) {
	cout << "Bloc " << i+1 << " with " << clustSize[i] << " nodes";
	cout << "\n\t  " << numCase[i] << " (" << percentCase << "%) Cases" << endl;
	cout << "\t  " << numCtrl[i] << " (" << percentCtrl << "%) Controls" << endl;
      }
      
      fprintf(output, "\nBloc %d with %d nodes\n\t%d Cases %.2f\%\n\t%d Controls %.2f\%\n", i+1, clustSize[i], numCase[i], percentCase, numCtrl[i], percentCtrl);
      
      for (int j = 0; j < clustSize[i]; j++) {
	int snpNum = clusts[i][j]; // number for this SNP
	
	if (snpNum >= numSnps) { // last allele alphabetically
	  snpNum -= numSnps; // get actual SNP number
	  
	  if (snpNum >= numSnps) // check for valid number
	    fatal("SNP number too high");
	  
	  fprintf(output,"%d\t%c\t",snpNum+1,(char)allelePairs[snpNum][1]);
	}
	
	else // first allele alphabetically
	  fprintf(output,"%d\t%c\t",snpNum+1,(char)allelePairs[snpNum][0]);
	
	for (int k = 0; k < numColsInfo; k++)
	  //fprintf(output,"%d\t",clusts[i][j]);		  
	  fprintf(output,"%s\t", ann[snpNum][k]);
	fprintf(output,"\n");
      }
    }
  }


  t.stop("Timer stopped.");
  cout << t << " seconds.\n" << endl;


  return 0;
}
