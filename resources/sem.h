#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#define PERMS (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_CREAT | O_EXCL)

int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
sem_t *sem_open(const char* name, int oflag, ...);
static sem_t* semlockp;
char SEMNAME[10] = "/checksum";
int MAXSEMS = 1;

int getnamed(char* name, sem_t** sem, int startval){
   while(((*sem=sem_open(name,FLAGS,PERMS,startval))==SEM_FAILED)&&(errno==EINTR));
   if(*sem!=SEM_FAILED)
      return 0;
   if(errno!=EEXIST)
      return -1;
   while(((*sem=sem_open(name,0))==SEM_FAILED)&&(errno==EINTR));
   if(*sem!=SEM_FAILED)
      return 0;
   return -1;
}

int semInit(char* name, int startval){
   if(getnamed(name,&semlockp,startval)==-1){
      return -1;
   }
   return 1;
}

int semWait(){
   while(sem_wait(semlockp)==-1)
      if(errno!=EINTR){
         return -1;
      }
   return 1;
}

int semSignal(){
   if(sem_post(semlockp)==-1){
      return -1;
   }
   return 1;
}

int semRelease(char* name){
   if((sem_unlink(name))==-1){
      return -1;
   }
   return 1;
}

