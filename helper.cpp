#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include "bloc.h"
#include "shmem.h"
#include "sem.h"

using namespace std;

int main(int argc, char ** argv)
{

  int numInd = atoi(argv[0]);
  int numSnps = atoi(argv[1]);
  int numNodes = atoi(argv[2]);
  int numSnps1 = atoi(argv[3]);
  int numSnps2 = atoi(argv[4]);
  int start1 = atoi(argv[5]);
  int start2 = atoi(argv[6]);
  float thresh = atof(argv[7]);
  int step = atoi(argv[8]);
  char * OUTPUT_FOLDER = (char *)malloc(sizeof(char) * 100);
  if(OUTPUT_FOLDER==NULL){
	fprintf(stderr,"error - could not allocate string\n");
	exit(1);
  }
  snprintf(OUTPUT_FOLDER,100,"%s",argv[9]);
  char * checkfileName = (char *)malloc(sizeof(char) * 100);
  if(checkfileName==NULL){
	fprintf(stderr,"error - could not allocate string\n");
	exit(1);
  }
  snprintf(checkfileName,100,"%s",argv[10]);
  int x1 = atoi(argv[11]);
  int x2 = atoi(argv[12]);
  int y1 = atoi(argv[13]);
  int y2 = atoi(argv[14]);
  char * outputfileName = (char *)malloc(sizeof(char) * 100);
  if(outputfileName==NULL){
	fprintf(stderr,"error - could not allocate string\n");
	exit(1);
  }
  snprintf(outputfileName,100,"%s/%s.gml",OUTPUT_FOLDER,argv[15]);
  vector<string> results;
  char result[100];
  unsigned long long int comparisons = 0;
  //char logfileName[200]; // hold name of logfile
  //sprintf(logfileName,"%s",argv[12]);

  getShm();
  if(semInit(SEMNAME,MAXSEMS) == -1){
	fprintf(stderr,"error - could not initialize semaphore\n");
	exit(1);
  }

  //FILE *logfile; // log file that records screen output

  //if ((logfile = fopen(logfileName, "w")) == NULL)
      //fatal("Log file could not be opened.\n");

  // write out nodes to output file
  //FILE *output;
  //FILE *edgefile; // use for edge IDs if PRINT_EDGE_IDS is set to 1

  //if ((output = fopen(outputfileName, "w")) == NULL)
      //fatal("Output file could not be opened.\n");

  // compute correlations and output edges

  float tally[4][4]; // tally number of each of 16 possible combinations
  float maxBloc = 0.0; // initialize for finding max and min values
  float minBloc = 1.0; 
  long int numEdges = 0; // tally number of edges printed out
  float minNoMissing = (float)numInd * NOMISS; // minimum of no missing relationships

  // adjust threshold to equal unscaled and unshifted value
  thresh = thresh / 4.5; // divide by 4.5 to get R_ij * ff_i * ff_j value

  // tally pairwise correlations
  for (int i = x1 - 1; i < x2 - 1; i++){ // start with each SNP in first set
    for (int j = y1 - 1; j < y2 - 1; j++){  // pair with each SNP in second set
      if(j <= i){//change to <
         continue;
      }
      ++comparisons;

      //fprintf(stdout,"helper %d snp1 %d snp2 %d i %d j %d comparisons %d yedge %d\n",getpid(),snp1,snp2,i,j,comparisons,yedge);

      //if (start1+i < start2+j) { // only compute upper diagonal of matrix

      // initialize values
      for (int row = 0; row < 4; row++)
	for (int col = 0; col < 4; col++)
	  tally[row][col] = 0;

      // add up number of individuals with each relationship      
      for (int k = 0; k < numInd; k++)
	tally[D->data1[i][k]][D->data2[j][k]]++; 

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

	//if(LOG_FILE)
	  //fprintf(logfile, "SNPs %d and %d have %d relationships without missing data.\nWarning: Correlation is based on too few relationships.\n\n", start1+i+1, start2+j+1, noMissing);
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
	ll *= D->freq1[i][0] * D->freq2[j][0]; // multiply by two frequency factors
	lh *= D->freq1[i][0] * D->freq2[j][1]; 
	hl *= D->freq1[i][1] * D->freq2[j][0]; 
	hh *= D->freq1[i][1] * D->freq2[j][1]; 
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

	  sprintf(result, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+1, weight);
          results.push_back(result);
	  //fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", i,j,weight);// changed from start1+i+1, start2+j+1, weight);
	  numEdges++;

	  /**if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+1)*numNodes + (start2+j+1));
	    fclose(edgefile);
	  }**/
	}

      if (TWONODE) {

	if(ll > thresh - TOL) { 

	  float weight = (ll * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");

	    //if(LOG_FILE)
	      //fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  }

	  sprintf(result, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+1, weight);
          results.push_back(result);
	  //fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+1, weight);
	  numEdges++;	

	  /**if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+1)*numNodes + (start2+j+1));
	    fclose(edgefile);
	  }**/
	}

	if(lh > thresh - TOL) { 
	  
	  float weight = (lh * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");
	    
	    
	    //if(LOG_FILE)
	      //fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  }

	  sprintf(result, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+numSnps+1, weight);
          results.push_back(result);
	  //fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+1, start2+j+numSnps+1, weight);
	  numEdges++;

	  /**if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+1)*numNodes + (start2+j+numSnps+1));
	    fclose(edgefile);
	  }**/
	}

	if(hl > thresh - TOL) { 
	  
	  float weight = (hl * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");

	    //if(LOG_FILE)
	      //fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  } 

	  sprintf(result, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+numSnps+1, start2+j+1, weight);
          results.push_back(result);
	  //fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+numSnps+1, start2+j+1, weight);
	  numEdges++;

	  /**if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+numSnps+1)*numNodes + (start2+j+1));
	    fclose(edgefile);
	  }**/
	}

	if(hh > thresh - TOL) { 
	  
	  float weight = (hh * 4.5);
	  if ((weight > 1.0 + TOL) || (weight < 0.0 - TOL)) {
	    cout << "\nWarning: CCC value is " << weight << endl;
	    //fatal("Invalid CCC value");
	    
	    //if(LOG_FILE)
	      //fprintf(logfile, "\nWarning: CCC value is %f\n", weight);
	  }

	  sprintf(result, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+numSnps+1, start2+j+numSnps+1, weight);
          results.push_back(result);
	  //fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", start1+i+numSnps+1, start2+j+numSnps+1, weight);
	  numEdges++;

	  /**if(PRINT_EDGE_IDS) {
	    if ((edgefile = fopen("edgeList.txt", "a")) == NULL)
	      fatal("'edgeList.txt' file could not be opened.\n");
	    fprintf(edgefile, "%d ", (start1+i+numSnps+1)*numNodes + (start2+j+numSnps+1));
	    fclose(edgefile);
	  }**/
	}
      }   
       
      // check that not too many edges are printed
      if(numEdges > MAX_NUM_EDGES){
	fatal("Too many edges printed out. Check MAX_NUM_EDGES in header file.");
      }
    //} // end of for (int j = i+1) loop
    }
  }

  if(results.size() > 0){
     	// write out nodes to output file
     	FILE *output;
     	if ((output = fopen(outputfileName, "w"))==NULL){
        	fatal("Output file could not be opened.\n");
	}
     	for(int x = 0;x < results.size();++x){
        	fprintf(output,"%s",results[x].c_str()); 
     	}
     	fclose(output);
  }

  if(comparisons > 0){
  	if(semWait()==1){
		// update comparisons
		FILE * read;
		if((read = fopen(checkfileName, "r")) == NULL){
			fprintf(stderr,"error - checksum file could not be opened\n");
			exit(1);
		}
		unsigned long long int comps = 0;
		fscanf(read,"%llu",&comps);
		comps += comparisons;
		fclose(read);
		FILE * write;
		if((write = fopen(checkfileName, "w"))==NULL){
			fprintf(stderr,"error - could not write to checksum file\n");
			exit(1);
		}
		fprintf(write,"%llu",comps);
		fclose(write);
		if(semSignal() == -1){
			fprintf(stderr,"error - could not signal semaphore\n");
			exit(1);
		}
  	}
  }

  free(OUTPUT_FOLDER);
  free(outputfileName);
  free(checkfileName);

}
