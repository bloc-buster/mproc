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
using namespace std;

int main(int argc,char ** argv){
	int ch;
	while((ch=getopt(argc,argv,"zc")) != -1){
		if(ch=='z' || ch=='c'){
			FILE * file = fopen("./params.h","r");
			if(file==NULL){
				printf("error - could not read params file\n");
				exit(1);
			}
			char buffer[200];
			char * token;
			char * gmlfile;
			char * tempfolder;
			char * granularity2;
			while(fgets(buffer,1000,file) != NULL){
				token = strtok(buffer," ");
				while(token != NULL){
					if(strcmp(token,"gml_file")==0){
						token = strtok(NULL," ");
						gmlfile = strdup(token);
						break;
					} else if(strcmp(token,"temp_folder")==0){
						token = strtok(NULL," ");
						tempfolder = strdup(token);
						break;
					} else if(strcmp(token,"granularity2")==0){
						token = strtok(NULL," ");
						granularity2 = strdup(token);
						break;
					}
					token = strtok(NULL," ");
				}
			}
			fclose(file);
			char command[150];
			if(ch=='z'){
				if(atoi(granularity2) > 0){
					sprintf(command,"./batch.sh %s", strtok(gmlfile,"\n"));
				} else {
					sprintf(command,"./ccc.sh %s %s", strtok(tempfolder,"\n"), strtok(gmlfile,"\n"));
				}
			} else if(ch=='c'){
				sprintf(command,"./ccc.sh %s %s", strtok(tempfolder,"\n"), strtok(gmlfile,"\n"));
			} else {
				fprintf(stderr,"error - unknown option\n");
				exit(1);
			}
			system(command);
			return 0;
		}
	}
        if(argc==5){
                char * outputfolder = argv[1];
                DIR * d = opendir(outputfolder);
                if(d==NULL){
                        printf("error - could not open output folder\n");
                        exit(1);
                }
                closedir(d);
                char * outputfile = argv[2];
                int numind = atoi(argv[3]);
                if(numind < 1){
                        printf("error - samples < 1\n");
                        exit(1);
                }
                int numsnps = atoi(argv[4]);
                if(numsnps < 1){
                        printf("error - snps < 1\n");
                        exit(1);
                }
                char command[150];
                sprintf(command,"./batch.sh %s %s %d %d",outputfolder,outputfile,numind,numsnps);
                system(command);
                return 0;
	} else if(argc < 8 || argc > 13){
		printf("\nusage\n");
		printf("\nprepare files: ./batch input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity1 (default 1) granularity2 (default 7) max_simultaneous_processes (default 15) output_folder (default temp_output_files) semaphores (0 or 1 default 0)\n");
		printf("\n(wait until jobs complete)\n");
		printf("\ncombine files: ./batch -z\n\n");
		exit(1);
	}
	char * input = argv[1];
	FILE * fptr = fopen(input,"r");
	if(fptr==NULL){
		printf("error - could not open input file\n");
		exit(1);
	}
	fclose(fptr);
	char * output = argv[2];
	float thresh = atof(argv[3]);
	if(thresh <= 0 || thresh > 1){
		printf("error - threshold must be between 0 and 1\n");
		exit(1);
	}
	int numind = atoi(argv[4]);
	if(numind < 1){
		printf("error - samples < 1\n");
		exit(1);
	}
	int numsnps = atoi(argv[5]);
	if(numsnps < 1){
		printf("error - snps < 1\n");
		exit(1);
	}
	int headerrows = atoi(argv[6]);
	if(headerrows < 0){
		printf("error - header rows < 0\n");
		exit(1);
	}
	int headercolumns = atoi(argv[7]);
	if(headercolumns < 0){
		printf("error - header columns < 0\n");
		exit(1);
	}
	int g1 = 1;
	if(argc >= 9){
		g1 = atoi(argv[8]);
		if(g1 < 1){
			printf("error - granularity < 1\n");
			exit(1);
		}
	}
	int g2 = 7;
	if(argc >= 10){
		g2 = atoi(argv[9]);
		if(g2 < 0){
			printf("error - granularity < 0\n");
			exit(1);
		}
		if(g1 > 1 && g2 > 7){
			printf("error - granularities larger than 7 may result in excessive processes generated\n");
			exit(1);
		}
		if(g1 > 7 && g2 > 0){
			printf("error - first granularity greater than 7 not recommended\n");
			exit(1);
		}
	}
	int procs = 15;
	if(argc >= 11){
		procs = atoi(argv[10]);
		if(procs < 1){
			printf("error - max processes < 1\n");
			exit(1);
		}
	}
	char * outputfolder = (char *)"../temp_output_files\0";
	if(argc >= 12){
		outputfolder = argv[11];
	}
	int semaphores = 0;
	if(argc >= 13){
		semaphores = atoi(argv[12]);
		if(semaphores != 1 && semaphores != 0){
			printf("error - semaphores must be equal to 0 or 1\n");
			exit(1);
		}
	}
	char command[150];
	sprintf(command,"srun ./batch.sh %s %s %f %d %d %d %d %d %d %d %s %d",input,output,thresh,numind,numsnps,headerrows,headercolumns,g1,g2,procs,outputfolder,semaphores);
	system(command);
	return 0;
}

