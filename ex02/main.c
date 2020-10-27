/* 
 * Primeiro exercicio do projeto de SO
 * Made by:
 * Duarte Jeremias, 96857
 * Rodrigo Liu, 96909
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <ctype.h>
#include "fs/operations.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100
#define TRUE 1
#define FALSE 0
#define MUTEX 1
#define RWLOCK 2
#define NOSYNC 3
#define CMD 4
#define RD 5
#define WR 6

int numberThreads = 0;

int lockType = 0;

pthread_mutex_t mutex;
pthread_mutex_t commandLock;
pthread_rwlock_t rwlock;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(char *inputFileName){
   char line[MAX_INPUT_SIZE];
   FILE *inputFile = fopen(inputFileName, "r");

   if(inputFile == NULL){
      fprintf(stderr, "Error: file %s does not exist\n", inputFileName);
      return;
   }

   /* break loop with ^Z or ^D */
   while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
      char token, type[MAX_INPUT_SIZE];
      char name[MAX_INPUT_SIZE];

      int numTokens = sscanf(line, "%c %s %s", &token, name, type);

      /* perform minimal validation */
      if (numTokens < 1) {
         continue;
      }
      switch (token) {
         case 'c':
            if(numTokens != 3)
               errorParse();
            if(insertCommand(line))
               break;
            return;

         case 'l':
            if(numTokens != 2)
               errorParse();
            if(insertCommand(line))
               break;
            return;

         case 'm':
            if(numTokens != 3)
               errorParse();
            if(insertCommand(line))
               break;
            return;

         case 'd':
            if(numTokens != 2)
               errorParse();
            if(insertCommand(line))
               break;
            return;

         case '#':
            break;

         default: { /* error */
            errorParse();
         }
      }
   }
   fclose(inputFile);
}

/*
 * Given a mode, this function locks the thread
 * Input: lock mode
 */
void lock(int mode){
   //if there isnt syncronization
   if(lockType == NOSYNC) return;

   //if lock necessary is the command lock
   else if(mode == CMD){
      if(pthread_mutex_lock(&commandLock)){
         fprintf(stderr, "Error: Mutex failed to lock\n");
         exit(EXIT_FAILURE);
      }
   }

   //if locktype is mutex
   else if(lockType == MUTEX){
      if(pthread_mutex_lock(&mutex)){
         fprintf(stderr, "Error: Mutex failed to lock\n");
         exit(EXIT_FAILURE);
      }
   }

   //if locktype is rwlock
   else if(lockType == RWLOCK){
      //if rwlock necessary is RD
      if(mode == RD){
         if(pthread_rwlock_rdlock(&rwlock)){
            fprintf(stderr, "Error: Rwlock failed to lock\n");
            exit(EXIT_FAILURE);
         }
      }
      //if rwlock necessary is WR
      if(mode == WR){
         if(pthread_rwlock_wrlock(&rwlock)){
            fprintf(stderr, "Error: Rwlock failed to lock\n");
            exit(EXIT_FAILURE);
         }
      }
   }
}

/*
 * Given a mode, this function unlocks the locked thread set earlier
 * Input: lock mode
 */
void unlock(int mode){
   //if there isnt syncronization
   if(lockType == NOSYNC) return; 

   //if lock is for the commands
   if(mode == CMD){ 
      if(pthread_mutex_unlock(&commandLock)){
         fprintf(stderr, "Error: Mutex failed to unlock\n");
         exit(EXIT_FAILURE);
      }
   }
   //if lock is mutex
   else if(lockType == MUTEX){
      if(pthread_mutex_unlock(&mutex)){
         fprintf(stderr, "Error: Mutex failed to unlock\n");
         exit(EXIT_FAILURE);
      }
   }
   //if lock is rwlock
   else if(lockType == RWLOCK){
      if(pthread_rwlock_unlock(&rwlock)){
         fprintf(stderr, "Error: rwlock failed to unlock\n");
         exit(EXIT_FAILURE);
      }
   }
}

void *applyCommands(void*ptr){
   while (TRUE){

      lock(CMD);
      if(numberCommands == 0){ //if command count is 0, thread should end
         unlock(CMD);
         break;
      }
      unlock(CMD);

      lock(CMD);
      const char* command = removeCommand();

      if (command == NULL){ //if command is NULL, thread should end
         unlock(CMD);
         break;
      }
      unlock(CMD);


      char token, type[MAX_INPUT_SIZE];
      char name[MAX_INPUT_SIZE];
      int numTokens = sscanf(command, "%c %s %s", &token, name, type);
      if (numTokens < 2) {
         fprintf(stderr, "Error: invalid command in Queue\n");
         exit(EXIT_FAILURE);
      }

      int searchResult;
      switch (token) {
         case 'c':
            switch (type[0]) {
               case 'f':
                  fprintf(stdout, "Create file: %s\n", name);
                  lock(WR); 
                  create(name, T_FILE);
                  unlock(0);
                  break;
               case 'd':
                  fprintf(stdout,"Create directory: %s\n", name);
                  lock(WR);
                  create(name, T_DIRECTORY);
                  unlock(0);
                  break;
               default:
                  fprintf(stderr, "Error: invalid node type\n");
                  exit(EXIT_FAILURE);
            }
            break;

         case 'l':
            lock(RD);
            searchResult = lookup(name);
            unlock(0);
            if (searchResult >= 0)
               fprintf(stdout, "Search: %s found\n", name);
            else
               fprintf(stdout, "Search: %s not found\n", name);
            break;

         case 'd':
            fprintf(stdout, "Delete: %s\n", name);
            lock(WR);
            delete(name);
            unlock(0);
            break;

         case 'm':
            fprintf(stdout, "Move: %s to %s\n", name, type);   
            lock(WR);
            move(name, type);
            unlock(0);
            break;

         default: { /* error */
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
         }
      }
   }
   return NULL;
}

/*
 * Parsing of the command line arguments
 * Input: argument count and arguments
 */
void argParse(int argc, char* argv[]){
   if(argc != 5)
      fprintf(stderr, "Error: Incorrect number of arguments given.\n");
   else if(!argv[1])
      fprintf(stderr, "Error: Input file given is NULL.\n");
   else if(!argv[2])
      fprintf(stderr, "Error: Output file given is NULL.\n");
   else if(atoi(argv[3]) <= 0)
      fprintf(stderr, "Error: Invalid number of threads given.\n");
   else if(!argv[4])
      fprintf(stderr, "Error: Synch method is NULL.\n");
   else {
      numberThreads = atoi(argv[3]);  //sets global var for the number of threads

      if(!strcmp(argv[4], "mutex")) lockType = MUTEX;  //if sync type is mutex

      else if(!strcmp(argv[4], "rwlock")) lockType = RWLOCK;  //if sync type is rwlock

      else if(!strcmp(argv[4], "nosync")){  //if sync type is nosync, there cant be more than 1 thread
         if(numberThreads > 1){ 
            fprintf(stderr, "Error: NoSync only available with 1 thread execution.\n");
            exit(EXIT_FAILURE);
         }
         lockType = NOSYNC;
      }
      else {
         fprintf(stderr, "Error: Invalid sync method.\n");
         exit(EXIT_FAILURE);
      }
      return;
   }
   exit(EXIT_FAILURE);
}

void init_lock(){
   if(lockType == NOSYNC) return; //if locktype is nosync, no need to initialize lock

   //if locktype is either mutex or rwlock, afterwards another mutex is initialized to acess the inputCommands vector
   if(lockType == MUTEX){ 
      if(pthread_mutex_init(&mutex, NULL)){
         fprintf(stderr, "Error: Mutex failed to init\n");
         exit(EXIT_FAILURE);
      }
   }
   else if(lockType == RWLOCK){
      if(pthread_rwlock_init(&rwlock, NULL)){
         fprintf(stderr, "Error: rwlock failed to init\n");
         exit(EXIT_FAILURE);
      }
   }
   if(pthread_mutex_init(&commandLock, NULL)){
      fprintf(stderr, "Error: Mutex failed to init\n");
      exit(EXIT_FAILURE);
   }
}

void destroy_lock(){
   if(lockType == NOSYNC) return; //if locktype is nosync, no need to destroy the lock

   if(lockType == MUTEX){
      if(pthread_mutex_destroy(&mutex)){
         fprintf(stderr, "Error: Failed destruction of mutex.\n");
         exit(EXIT_FAILURE);
      }
   }
   else if(lockType == RWLOCK){
      if(pthread_rwlock_destroy(&rwlock)){
         fprintf(stderr, "Error: Failed destruction of rwlock.\n");
         exit(EXIT_FAILURE);
      }
   }
   if(pthread_mutex_destroy(&commandLock)){
      fprintf(stderr, "Error: Failed destruction of mutex.\n");
      exit(EXIT_FAILURE);
   }
}

int main(int argc, char* argv[]) {
   argParse(argc, argv);

   struct timeval startTime, endTime;
   int i;

   pthread_t threadPool[numberThreads];
   gettimeofday(&startTime, NULL);

   /* init filesystem */
   init_fs();

   init_lock();

   /* process input and print tree */
   processInput(argv[1]);

   if(lockType == NOSYNC){
      applyCommands(NULL);
   } else {
      for(i = 0; i < numberThreads; i++){
         if(pthread_create(&threadPool[i], NULL, applyCommands, NULL)){
            fprintf(stderr, "Error: thread creation failed.\n");
            exit(EXIT_FAILURE);
         }
      }
      for(i = 0; i < numberThreads; i++){
         if(pthread_join(threadPool[i], NULL)){
            fprintf(stderr, "Error: thread merging failed.\n");
            exit(EXIT_FAILURE);
         }
      }
   }

   print_tecnicofs_tree(argv[2]);

   gettimeofday(&endTime, NULL);

   destroy_lock();


   /* release allocated memory */
   destroy_fs();

   printf("TecnicoFS completed in %.4f seconds.\n", (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6);

   exit(EXIT_SUCCESS);
}
