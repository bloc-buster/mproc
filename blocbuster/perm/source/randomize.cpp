/****************************************************************************
*
*	randomize.cpp:	Code for randomizing genotype data.
*                       Each row will be independently randomized 
*                      
*      
*
*                       Sharlee Climer
*                       January 2011
*
*       Modified to read in only one row at a time to reduce memory
*       requirements.  Sharlee Climer, May 2012.
*       
*                       
****************************************************************************/
  

#include "randomize.h"

using namespace std;

void randomize(char*, char*, int, int, int, int); // read in and randomize input data 


int main(int argc, char ** argv)
{
  if (argc != 7)
    fatal("Usage:\n\n   perm input.txt output.txt numDataCols numDataRows numHeadCols numHeadRows\n\n");  

  timer t;
  t.start("Timer started.");

  cout << "\nCommand line arguments: \n\t";
  for (int i = 0; i < argc; i++)
	cout << argv[i] << " ";
  cout << "\n" << endl;

  cout << "*** Important: Assuming rows represent markers and " << endl;
  cout << "    columns represent individuals in input file." << endl;
  cout << "    Each genotype must be represented as a single " << endl;
  cout << "    string with no white space. ***\n" << endl;

  char strng[200]; // temporary string storage

  int numInd = atoi(argv[3]); // number of individuals
  int numMark = atoi(argv[4]);  // number of markers
  int numHeadCols = atoi(argv[5]); // number of header columns
  int numHeadRows = atoi(argv[6]); // number of header rows

  cout << numInd << " individuals and " << numMark << " markers." << endl;
  cout << "Assuming " << numHeadRows << " header rows and " << numHeadCols << " header columns." << endl;
  cout << "Assuming input data strings have no more than " << SIZESTRNG << " characters each and " << endl;
  cout << " header strings have no more than " << SIZESTRNGHEAD << " characters each.\n" << endl;

  randomize(argv[1], argv[2], numInd, numMark, numHeadCols, numHeadRows);

  t.stop("\nTimer stopped.");
  cout << t << " seconds.\n" << endl;

  return 1;
}



void randomize(char* inFile, char* outFile, int numInd, int numMark, int numHeadCols, int numHeadRows) // randomize input data
{
  if (!QUIET)
    cout << "\nReading in data...\n" << endl;

  FILE *input;
  FILE *output;
  char strng[SIZESTRNGHEAD]; // temporary string storage
  
  if ((input = fopen(inFile, "r")) == NULL)
    fatal("Input file could not be opened.\n");

  if ((output = fopen(outFile, "w")) == NULL)
    fatal("Output file could not be opened.\n");

  // prepare for randomization
  pid_t pidNum; // hold process ID number to use for seed
  pidNum = getpid(); 

  srand((int)pidNum); // set random seed

  //cout << "\nRandom seed = " << pidNum << "\n\n" << endl;

  int randomVal; // random number
  
  // read in header rows and write out
  for (int i = 0; i < numHeadRows; i++) {
    for (int j = 0; j < numHeadCols + numInd; j++) {

     fscanf(input, "%s", strng); 

     //cout << i << ", " << j << ": " << strng << endl;
      
      if(feof(input))
	fatal("Input file is missing data");
      
      fprintf(output, "random ");
    }

    fprintf(output,"\n");
  }

  // read in data and header columns
  
  // allocate data array memory
  char **data;
  if ((data = new char*[numInd]) == NULL)
    fatal("memory not allocated");
  for (int i = 0; i < numInd; i++) 
    if ((data[i] = new char[SIZESTRNG]) == NULL)
      fatal("memory not allocated");
    
 // read in data one row at a time
  for (int i = 0; i < numMark; i++) {
    for (int j = 0; j < numHeadCols; j++) {
      fscanf(input, "%s", strng); 
      if(feof(input))
	fatal("Input file is missing data");
      fprintf(output,"%s ", strng);
    }

    for (int j = 0; j < numInd; j++) {
      fscanf(input, "%s", data[j]); 
      if(feof(input))
	fatal("Input file is missing data");
    }

    // randomize row 
    for (int j = 0; j < numInd; j++) { //assign random data for each individual
      // assign random value between 0 and numInd-1
      randomVal = rand() % numInd;
      
      while(data[randomVal][0] == '\0') // check for data already taken
	randomVal = rand() % numInd;

      fprintf(output, "%s ", data[randomVal]); // print out random data
      data[randomVal][0] = '\0'; // delete this value
    }

    fprintf(output, "\n");
  }

  // check for end of file
  if (!feof(input)){
    fscanf(input, "%s", strng);
    if (!feof(input))
      fatal("Unread data in input file");
  }

  fclose(input);
  fclose(output);

}
