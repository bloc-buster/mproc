#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include "params.h"

typedef struct{
  char data1[num_snps1][num_ind];  // hold first set of genotypes
  char data2[num_snps2][num_ind];  // hold second set of genotypes
  double freq1[num_snps1][2]; // hold frequency values for alleles for first SNP set
  double freq2[num_snps2][2]; // hold frequency values for alleles for second SNP set
} DATA;

DATA* D;
int data_id;
//char DATAKEYNAME[] = "./data.key";

void getShm(){
   FILE* dataptr;
   if((dataptr=fopen(DATAKEYNAME,"w"))==NULL){
      fprintf(stderr,"shmem.c could not find data key file");
      exit(1);
   }
   fclose(dataptr);
   key_t datakey = ftok(DATAKEYNAME,1);
   if(datakey==(key_t)-1){
      fprintf(stderr,"shmem.c could not initialize data key");
      exit(1);
   }
   data_id = shmget(datakey,sizeof(DATA),IPC_CREAT | 0660);
   D = (DATA*)shmat(data_id,NULL,0);
   if(D==(void*)-1){
      fprintf(stderr,"shmem.c could not initialize shared data");
      exit(1);
   }
}

void initShm(){

  if ((num_snps1 < 1) || (num_snps2 < 1))
        fatal("Number of SNPs in set is less than 1");

  /* 
  if (((D->data1 = new char* [num_snps1]) == NULL) || ((D->data2 = new char* [num_snps2]) == NULL))
    fatal("Memory not allocated");

  for (int i = 0; i < num_snps1; i++)
    if ((D->data1[i] = new char[num_ind]) == NULL)
      fatal("Memory not allocated");

  for (int i = 0; i < num_snps2; i++)
    if ((D->data2[i] = new char[num_ind]) == NULL)
      fatal("Memory not allocated");
  */

  // initialize values to '4' as valid values are 0, 1, 2, or 3
  for (int i = 0; i < num_snps1; i++)
    for (int j = 0; j < num_ind; j++)
      D->data1[i][j] = '4';

  for (int i = 0; i < num_snps2; i++)
    for (int j = 0; j < num_ind; j++)
      D->data2[i][j] = '4';

  /*
  if ((D->freq1 = new double*[num_snps1]) == NULL)
    fatal("memory not allocated");

  for (int i = 0; i < num_snps1; i++)
    if ((D->freq1[i] = new double[2]) == NULL)
      fatal("memory not allocated");

  if ((D->freq2 = new double*[num_snps2]) == NULL)
    fatal("memory not allocated");

  for (int i = 0; i < num_snps2; i++)
    if ((D->freq2[i] = new double[2]) == NULL)
      fatal("memory not allocated");
  */

  // initialize frequency values to zero
  for (int i = 0; i < num_snps1; i++)
    for (int j = 0; j < 2; j++)
      D->freq1[i][j] = 0;

  for (int i = 0; i < num_snps2; i++)
    for (int j = 0; j < 2; j++)
      D->freq2[i][j] = 0;


}

void releaseShm(){
   shmdt(D);
   shmctl(data_id,IPC_RMID,NULL);
}


