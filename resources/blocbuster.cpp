#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <iostream>
#include <cmath>
using namespace std;

// usage description
void instructions(){
		printf("\nusage\n");
		printf("\nprepare files: ./blocbuster input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity1 (default 1) granularity2 (default 7) max_simultaneous_processes (default 15) output_folder (default log_files) semaphores (0 or 1 default 0)\n");
		printf("\noptions: -g # (granularity 1) -G # (granularity 2) -h (help) -n (quantify resulting processes or jobs) -o name (output folder name) -p # (max processes) -s (semaphores, 0 yes 1 no) -z (second run)\n");
		printf("\n(wait until jobs complete, then perform second run)\n");
		printf("\ncombine files: ./blocbuster -z\n\n");
		exit(1);
}

// quantify jobs
int formula1(int n){
	float N = ((float)n * (float)(n+ 1)) / 2.0;
	return (int)N;
}

// quantify processes (jobs * procs per job)
int formula2(int g1, int g2){
	return (g1 * formula1(g2)) + (formula1(g1 - 1) * pow(g2,2));
}
 
// reduction of formula 2
// p = mn(mn+1) / 2
int formula3(int n, int m){
	return (int) ( ( (n * m) * (n * m + 1) ) / 2 );
}

int main(int argc,char ** argv){
	// get working directory from which program was invoked
	char dir[MAXPATHLEN+1];
	getcwd(dir,MAXPATHLEN+1);
	// get absolute path of program (blocbuster executable file)
	char * root = realpath(argv[0],NULL);
	// remove "blocbuster" from path, leaving root path remaining
	char root1[100];
	//bool found = false;
        for (int i = strlen(root) - 1; i >= 0; i--) {
           if (root[i] == '/') {
	      //found = true;
              strncpy(root1,root,i+1);
	      root1[i+1] = '\0';
              break;
           }
        }
	// get command line options
	bool computeN = false;
	int options = 0;
	int g1 = 1;
	int g2 = 7;
	int procs = 15;
	char * log_folder = (char *)"log_files";
	int semaphores = 0;
	int ch;
	while((ch=getopt(argc,argv,"g:G:hno:p:s:z")) != -1){
		// granularity 1, default 1
		if(ch=='g'){
			g1 = atoi(optarg);
			++options;
			++options;
		// granularity 2, default 7
		} else if(ch=='G'){
			g2 = atoi(optarg);
			++options;
			++options;
		// help
		} else if(ch=='h'){
			instructions();
		// quantify processes
		} else if(ch=='n'){
			computeN = true;
			++options;
		// output folder name (default log_files)
		} else if(ch=='o'){
			log_folder = optarg;
			++options;
			++options;
		// max processes (default 15)
		} else if(ch=='p'){
			procs = atoi(optarg);
			++options;
			++options;
		// semaphores (default 0 = no, 1 = yes)
		} else if(ch=='s'){
			semaphores = atoi(optarg);
			++options;
			++options;
		// second run, merge partial gml files
		} else if(ch=='z'){
			// build absolute path to params.h configuration file, main.sh, and ccc.sh
			char paramfile[150];
			char mainfile[150];
			char cccfile[150];
			sprintf(paramfile,"%sparams.h\0",root1);
			sprintf(mainfile,"%smain.sh\0",root1);
			sprintf(cccfile,"%sccc.sh\0",root1);
			// validate able to read params file
			FILE * file = fopen(paramfile,"r");
			if(file==NULL){
				printf("error - could not read params file\n");
				exit(1);
			}
			// read variables and paths from params file
			char buffer[200];
			char * token;
			char * gmlfile;
			char * tempfolder;
			char * granularity1;
			char * granularity2;
			char * prevaddress;
			while(fgets(buffer,1000,file) != NULL){
				token = strtok(buffer," ");
				while(token != NULL){
					// absolute path to gml file
					if(strcmp(token,"gml_file")==0){
						token = strtok(NULL," ");
						gmlfile = strdup(token);
						break;
					// absolute path to temp output folder
					} else if(strcmp(token,"temp_folder")==0){
						token = strtok(NULL," ");
						tempfolder = strdup(token);
						break;
					// granularity 1 value
					} else if(strcmp(token,"granularity1")==0){
						token = strtok(NULL," ");
						granularity1 = strdup(token);
						break;
					// granularity 2 value
					} else if(strcmp(token,"granularity2")==0){
						token = strtok(NULL," ");
						granularity2 = strdup(token);
						break;
					// absolute root path to executables
					} else if(strcmp(token,"root")==0){
						token = strtok(NULL," ");
						// current working directory may have changed, but previous absolute path was saved and will still work
						prevaddress = strdup(token);
						break;
					}
					token = strtok(NULL," ");
				}
			}
			fclose(file);
			// remove newline from end of root executable path
			prevaddress[strlen(prevaddress) - 1] = '\0';
			// build command
			char command[150];
			// if using multiprocessing
			if(atoi(granularity2) > 0){
				// combined batch processing (SLURM) + multiprocessing
				// remove newline from end of temp output folder path with strtok, specifies path for slurm output file
				if(atoi(granularity1) > 0){
					sprintf(command,"srun %smain.sh %s", prevaddress, strtok(tempfolder,"\n"));
				// only multiprocessing (non-HPC, probable Linux cloud virtual machine)
				// tempfolder parameter not used by next file except to identify number of args
				} else {
					sprintf(command,"%svirt_main.sh %s", prevaddress, strtok(tempfolder,"\n"));
				}
			// otherwise only batch processing
			} else {
				// remove newline from end of temp output folder path
				char * temp = strtok(tempfolder,"\n");
				temp[strlen(temp) - 1] = '\0';
				sprintf(command,"sbatch --output=%s%%j.out\" %s %s\" %s", temp, cccfile, temp, strtok(gmlfile,"\n"));
			}
			system(command);
			// attempted to return fail if checksum incorrect
			//FILE* f = popen("echo $?", "r");
			//int errcode;
			//fscanf(f,"%d",&errcode);
			//pclose(f);
			//printf("returning code %d\n",errcode);
			return 0;
		}
	}
	// validate number of args
	if(argc - options < 8 || argc - options > 13){
		instructions();
	}
	// add flags to argv index, otherwise argv includes flags with args
	int argindex = 1 + options;
	// get absolute path of data input file
	char * inputfile = realpath(argv[argindex++], NULL);
	// validate input file is readable
	FILE * fptr = fopen(inputfile,"r");
	if(fptr==NULL){
		printf("error - could not open input file %s\n",argv[argindex - 1]);
		exit(1);
	}
	fclose(fptr);
	// find root path to data input file for placing of output file and temp output folder
	// read from end of path until first forward slash, then truncate path
  	char filestem[100];
        for (int i = strlen(inputfile) - 1; i >= 0; i--) {
             if (inputfile[i] == '/') {
		strncpy(filestem,inputfile,i+1);
		filestem[i+1] = '\0';
                break;
             }
	}
	// if input file is in same folder as executables, request alternative
	if(strcmp(root1,filestem)==0){ 
		fprintf(stderr,"error - please place input file in a different folder than the executables\n");
		exit(1);
	}
	// output file name without path
	char * output = argv[argindex++];
	// validate output file name ends with .gml
	char * substr = strstr(output,".gml");
	if(substr==NULL || strcmp(substr,".gml") != 0){
		fprintf(stderr,"error - output file must end with .gml, %s\n",output);
		exit(1);
	}
	// validate output file is just file name, not file path
	if(strstr(output,"/") != NULL){
		fprintf(stderr,"error - outputfile must be a file name, not a file path\n");
		fprintf(stderr,"%s\n",output);
		exit(1);
	}
	// prepend root input file path to output file name
	char outputfile[strlen(filestem) + strlen(output) + 1];
	sprintf(outputfile,"%s%s\0",filestem,output);
	// threshold of significance (0..1)
	float thresh = atof(argv[argindex++]);
	if(thresh <= 0 || thresh > 1){
		printf("error - threshold must be between 0 and 1\n");
		exit(1);
	}
	// number of individual subjects in experimental data
	int numind = atoi(argv[argindex++]);
	if(numind < 1){
		printf("error - samples < 1\n");
		exit(1);
	}
	// number of snps in experimental data
	int numsnps = atoi(argv[argindex++]);
	if(numsnps < 1){
		printf("error - snps < 1\n");
		exit(1);
	}
	// number of header rows in data input file
	int headerrows = atoi(argv[argindex++]);
	if(headerrows < 0){
		printf("error - header rows < 0\n");
		exit(1);
	}
	// number of header columns in data input file
	int headercolumns = atoi(argv[argindex++]);
	if(headercolumns < 0){
		printf("error - header columns < 0\n");
		exit(1);
	}
	// begin optional args with default values
	if(argc - options >= 9){
		// first layer granularity value for batch processing (default 1)
		g1 = atoi(argv[argindex++]);
		if(g1 < 0){
			printf("error - granularity < 0\n");
			exit(1);
		} else if(g1 > numsnps){
			printf("error - granularity greater than snps\n");
			exit(1);
		}
	}
	if(argc - options >= 10){
		// second layer granularity value for multiprocessing (default 7)
		g2 = atoi(argv[argindex++]);
		if(g2 < 0){
			printf("error - granularity < 0\n");
			exit(1);
		} else if(g1 > 0 && g2 > numsnps / g1){
			printf("error - granularity 2 greater than snps divided by granularity 1\n");
			exit(1);
		}
	}
	if(g1 <= 0 && g2 <= 0){
		printf("error - must have at least one granularity value\n");
		exit(1);
	}
	// if -n flag was specified
	if(computeN==true){
		if(g1==0){
			// just quantify multiprocessing
			printf("processes %d\n",formula1(g2));
		} else if(g2==0){
			// just quantify batch processing
			printf("jobs %d\n",formula1(g1));
		} else {
			// combined batch and multiprocessing, quantify total processes from all jobs
			printf("processes %d\n", formula3(g1,g2));
		}
		exit(0);
	}
	// maximum processes per node (default 15)
	if(argc - options >= 11){
		procs = atoi(argv[argindex++]);
		if(procs < 1){
			printf("error - max processes < 1\n");
			exit(1);
		}
	}
	// output folder name without path (default log_files)
	if(argc - options >= 12){
		log_folder = argv[argindex++];
		if(strstr(log_folder,"/") != NULL){
			fprintf(stderr,"error - temp output folder must be a folder name, not a folder path\n");
			exit(1);
		}
	}
	// prepend data input file root path to output folder name
	char outputfolder[strlen(filestem) + strlen(log_folder) + 1];
	sprintf(outputfolder,"%s%s/\0",filestem,log_folder);
	// semaphores (default 0 = non, 1 = yes)
	if(argc - options >= 13){
		semaphores = atoi(argv[argindex++]);
		if(semaphores < 0 || semaphores > 1){
			printf("error - semaphores must be equal to 0 or 1\n");
			exit(1);
		}
	}
	// determine whether command to run is for HPC system (batch processing) or virtual cloud machine (only multiprocessing)
	char command[150];
	if(g1 > 0){
		// HPC system, batch processing + optional multiprocessing
		sprintf(command,"srun %smain.sh %s %s %f %d %d %d %d %d %d %d %s %d",root1,inputfile,outputfile,thresh,numind,numsnps,headerrows,headercolumns,g1,g2,procs,outputfolder,semaphores);
	} else {
		// virtual cloud machine, only multiprocessing
		sprintf(command,"%svirt_main.sh %s %s %f %d %d %d %d %d %d %d %s %d",root1,inputfile,outputfile,thresh,numind,numsnps,headerrows,headercolumns,g1,g2,procs,outputfolder,semaphores);
	}
	system(command);
	return 0;
}

