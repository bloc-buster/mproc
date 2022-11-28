/****************************************************************************
*
*	keepHighWt.cpp:	Code for retaining edges with high weights. 
*                 
*
*                       Sharlee Climer
*                       June 2011
*
*
****************************************************************************/
  

#include "keepHighWt.h"

using namespace std;

int main(int argc, char ** argv)
{
  if (argc != 6)
    fatal("\n\nUsage:\n  keepHi input.gml numNodes numEdgesOrig numEdgesKeep output.gml\n\n"); 

  timer t;
  t.start("Timer started.");

  cout << "\nCommand line arguments: \n\t";
  for (int i = 0; i < argc; i++)
        cout << argv[i] << " ";
  cout << "\n" << endl;

  FILE *input;
  FILE *output;
  
  if ((input = fopen(argv[1], "r")) == NULL) 
    fatal("File could not be opened.\n");

  long int numNodes = atoi(argv[2]); // number of nodes
  long int numEdges = atoi(argv[3]); // number of edges in original network
  long int numKeep = atoi(argv[4]); // number of edges to keep 
  long int numKept = 0; // actual number of edges kept (may be different due to ties)
  int numTies = 0; // number of edges added due to ties
  long int countNodes = 0; // count nodes for data checking
  long int countEdges = 0; // count edges for data checking
  char strng[200]; // temporary string storage

  // check validity of values
  if (numNodes < 1)
    fatal("Network must have at least one node");
  if (numEdges < 1)
    fatal("Network must have at least one edge");
  if (numKeep < 1)
    fatal("Must require at least one edge is kept");
  if (numKeep > numEdges)
    fatal("Cannot keep more edges than those that exist in the original network");

  cout << "'" << argv[5] << "' will hold new network with " << numKeep << " highest-weight edges." << endl;

  long int min = 1000000000;   // hold min node number
  long int max = -1000000000;  // hold max node number

  int startOne; // 1 if start node number is 1, 0 if start number is 0

  while (1) { // read in until get to "graph" declaration
    if(feof(input)) fatal("no graph to read in input file");

    fscanf(input, "%s", strng); // read in string from file

    if(strncmp(strng, "graph", 5) == 0) 
      break; // stop when get to "graph" declaration
  }

  while (1) { // throw away everything until get to "edge" declaration
    if(feof(input)) fatal("no edges to read in input file");

    fscanf(input, "%s", strng); // read in string from file

    if(strncmp(strng, "id", 2) == 0) { 
      fscanf(input, "%s", strng); // read in id value
      int num = atoi(strng);

      if(min > num) min = num; // update minimum node number
      if(max < num) max = num; // update maximum node number
      countNodes++; // one more node found
    }

    if(strncmp(strng, "edge", 4) == 0) 
      break; // stop when read in first "edge"
  }

  if(!QUIET)
    cout << "\nNode numbers range from " << min <<" to " << max << "." << endl;

  // check number of nodes
  if(countNodes != numNodes)
    fatal("Input file does not contain specified number of nodes.");

  if(!QUIET)
    cout << "Reading in graph from '" << argv[1] << "'...\n" << endl;

  startOne = min;

  // check validity of values
  if(startOne < 0) fatal("Node numbers can not be negative");

  if ((startOne != 0) && (startOne != 1)) 
    fatal("Smallest node number should be 0 or 1");

  if(max != numNodes - 1 + startOne)
    fatal("Error on maximum node number");

  float *weights; 
  if ((weights = new float[numEdges]) == NULL)
    fatal("memory not allocated");

  // read in edge weights

  while (!feof(input)) { // read edges until reach end of file
    fscanf(input, "%s", strng); // read in string from file

    if(!strncmp(strng, "[", 1) == 0) // check for correct character
      fatal("No '[' after edge declaration");

    fscanf(input, "%s", strng); // read in string from file
    if(!strncmp(strng, "source", 6) == 0) // check for correct character
      fatal("No 'source' declaration");

    fscanf(input, "%s", strng); // read in start node from file
    int source = atoi(strng);

    if((source < min) || (source > max))
      fatal("Invalid node number");

    //cout << "source: " << idInv[source];

    fscanf(input, "%s", strng); // read in string from file
    if(!strncmp(strng, "target", 6) == 0) // check for correct character
      fatal("No 'target' declaration");

    fscanf(input, "%s", strng); // read in target node from file
    int target = atoi(strng);

    if((target < min) || (target > max))
      fatal("Invalid node number");

    //cout << ", target: " << idInv[target];

    fscanf(input, "%s", strng); // read in string from file

    if(strncmp(strng, "weight", 6) == 0) {// check to see if weight specified
      fscanf(input, "%s", strng); // read in string from file
      weights[countEdges] = atof(strng);
      fscanf(input, "%s", strng); // read in string from file
    }
    else
      fatal("All edges must have a weight specified");

    //cout << ", weight: " << weight << endl;
    
    countEdges++; // count number of edges

    if(countEdges % 10000000 == 0) // message every 10 million edges
      cout << countEdges / 1000000 << " million edges read" << endl;

    if(!strncmp(strng, "]", 1) == 0) // check for correct character
      fatal("No ']' after edge declaration");

    //fprintf(output, "%d\t%d\t%f\n", source, target, weight);

    //if (feof(input)) break; // break as at end of file

    fscanf(input, "%s", strng); // read in string from file

    if(strncmp(strng, "]", 1) == 0) // check for end of graph
      break;

    if(!strncmp(strng, "edge", 4) == 0) // check for correct character
      fatal("No 'edge' declaration");
  }

  fclose(input);

  if(countEdges != numEdges)
    fatal("Incorrect number of edges in input file");

  if (VERBOSE) {
    cout << "Weights of original edges:" << endl;
    for (int i = 0; i < numEdges; i++)
      cout << weights[i] << " ";
    cout << "\n" << endl;
  }

  std::sort(weights, weights + numEdges); // sort data using quicksort

  if (VERBOSE) {
    cout << "Sorted edge weights:" << endl;
    for (int i = 0; i < numEdges; i++)
      cout << weights[i] << " ";
    cout << "\n" << endl;
  }

  // determine minimum weight edge to keep
  float minWt = weights[numEdges - numKeep]; // values are in increasing order
  minWt -= 0.00001; // capture edges within tolerance level 

  cout << "Edges with weight of " << minWt << " or higher will be kept." << endl;

  // read in old edges and write out ones to keep

  if (((input = fopen(argv[1], "r")) == NULL) || ((output = fopen(argv[5], "w")) == NULL))
    fatal("File could not be opened.\n");

  fprintf(output, "Graph with %d nodes.\ngraph\n[\n",numNodes);

  while (1) { // read in until get to "graph" declaration
    if(feof(input)) fatal("no graph to read in input file");

    fscanf(input, "%s", strng); // read in string from file

    if(strncmp(strng, "graph", 5) == 0) 
      break; // stop when get to "graph" declaration
  }

  // write out node IDs
  for (int i = startOne; i < numNodes+startOne; i++) 
    fprintf(output,"\tnode\n\t[\n\tid %d\n\t]\n", i);

  while (1) { // throw away everything until get to "edge" declaration
    if(feof(input)) fatal("no edges to read in input file");

    fscanf(input, "%s", strng); // read in string from file

    if(strncmp(strng, "edge", 4) == 0) 
      break; // stop when read in first "edge"
  }

  int source; // hold copy of current endpoints
  int target; 

  while(!feof(input)) { // read in rest of file
    for (int i = 0; i < 3; i++)
      fscanf(input, "%s", strng); // read in '[ source' and source number

    source = atoi(strng); // source of current edge

    for (int i = 0; i < 2; i++)
      fscanf(input, "%s", strng); // read in 'target' and target number

    target = atoi(strng); // target of current edge

    // check for weight, end of file, or another edge
    fscanf(input, "%s", strng); // read in either 'weight' or ']'
    float weight;

    if(strncmp(strng, "weight", 6) == 0) {// check to see if weight specified
      fscanf(input, "%s", strng); // read in string from file
      weight = atof(strng);
      fscanf(input, "%s", strng); // read in ']' from file
    }

    if(feof(input))
      fatal("Input file has missing data or improper format");

    // print out edge if edge is not to be deleted
    if(weight > minWt-TOL) {
      fprintf(output, "\tedge\n\t[\n\tsource %d\n\ttarget %d\n\tweight %f\n\t]\n", source, target, weight);
      numKept++;
      //cout << source << ", " << target << endl;
    }

    fscanf(input,"%s", strng); // read in either 'edge' or final ']'

    if(strncmp(strng, "]", 1) == 0) // end of file
      break;
  }

  fprintf(output,"]\n"); // write out final bracket

  fclose(input);
  fclose(output);




  cout << numKept << " edges written to '" << argv[5] << "' (" << numKept-numKeep << " retained due to ties).\n" << endl;

  t.stop("Timer stopped.");
  cout << t << " seconds." << endl;

  return 1;
}

