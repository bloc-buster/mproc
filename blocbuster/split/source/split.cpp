/**
* This program will take one file and randomly select half the lines to put in
* the first output file and put the other half in the other output file, or
* optionally output a specified number of lines to the first file.
*
* Peter Holly, June 2011
*/

#include <iostream>
#include <fstream>
#include <time.h>
#include <set>
#include <string>
#include <stdlib.h>

using namespace std;

bool isWhiteSpace(std::string line) {
  for(int i = 0; i < line.size(); i++) {
    if(!isspace(line[i])) {
      return false;
    }
  }
  return true;
}

int main(int argc, const char* argv[]) {
  // check if correct number of file name have been input
  if((argc != 5) && (argc != 6)) {
    std::cout << "Usage: split input.txt output1.txt output2.txt numberOfHeaderRows (numberOfLinesInOutput1)" << std::endl;
    return 0;
  }

  cout << "\n  *** IMPORTANT: Rows will be split between the two output files! ***\n" << endl;

  int numHeaderRows(atoi(argv[4]));
  int numLines(0);
  int count; //counts number of lines that are written out
  int numOutPutLines; //this will equal number of lines for the first line if this is specified, otherwise it will be null
  bool done(false); //temp variable to deterimine when loop is done
  std::string line; //Temp used while reading in and copying back out each line
  std::set<int> lineNums; //stores the line numbers for the lines selected for first output file
  std::fstream inputFile(argv[1]); 
  std::ofstream outputFile1(argv[2]);
  std::ofstream outputFile2(argv[3]);

  // check if all files exist or could be opened
  if(!inputFile.is_open() || !outputFile1.is_open() || !outputFile2.is_open()) {
    std::cout << "Unable to open file" << std::endl;
    return EXIT_FAILURE;
  }

  // count number of lines
  while(inputFile.good() && !done) {
    getline(inputFile, line);
    if(!isWhiteSpace(line)) {
      numLines++;
    } else {
      done = true;
    }
  }
  inputFile.close();

  if(argv[5] == NULL) {
    numOutPutLines = (numLines - numHeaderRows) / 2;
  } else {
    numOutPutLines = atoi(argv[5]);
  }

  // Generate random line numbers
  srand((unsigned)time(NULL));
  while(lineNums.size() != numOutPutLines) {
    lineNums.insert(rand() % (numLines - numHeaderRows) + numHeaderRows);
  }

  // Output lines to output files
  inputFile.open(argv[1]);
  for(count = 0; count < numLines && inputFile.good() && outputFile1.good() && outputFile2.good(); count++) {
    getline(inputFile, line);
    if(count < numHeaderRows) {
      outputFile1 << line << std::endl;
      outputFile2 << line << std::endl;
    } else if(lineNums.count(count)) {
      outputFile1 << line << std::endl;
    } else {
      outputFile2 << line << std::endl;
    }
  }

  // Check that correct number of lines were written out
  if(count != numLines) {
    std::cout << "Error in reading or writing files" << std::endl;
    return EXIT_FAILURE;
  }

  inputFile.close();
  outputFile1.close();
  outputFile2.close();

  return 0;
}



