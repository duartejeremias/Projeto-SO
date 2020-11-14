/* 
 * Segundo exercicio do projeto de SO
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

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100
#define TRUE 1
#define FALSE 0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t podeProd;
pthread_cond_t podeCons;

int numberThreads = 0;
int numberCommands = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int headQueue = 0;
int existingCommands = 0;

int insertCommand(char* data) {
   if(existingCommands < MAX_COMMANDS) {
      existingCommands++;
      
      int i = numberCommands;
      numberCommands++;
      if(numberCommands == MAX_COMMANDS) numberCommands = 0;
      strcpy(inputCommands[i], data);
      return 1;
   }
   return 0;
}

char* removeCommand() {
   if(existingCommands > 0){
      
      int i = headQueue;
      if(strcmp(inputCommands[i], "quit") == 0){
         return inputCommands[i];
      }
      existingCommands--;
      headQueue++;
      if(headQueue == MAX_COMMANDS) headQueue = 0;

      return inputCommands[i];
   }
   return NULL;
}

void errorParse(){
   fprintf(stderr, "Error: command invalid\n");
   exit(EXIT_FAILURE);
}

void *processInput(void *ptr){
   char *inputFileName = (char*) ptr;
   char line[MAX_INPUT_SIZE];
   FILE *inputFile = fopen(inputFileName, "r");

   if(inputFile == NULL){
      fprintf(stderr, "Error: file %s does not exist\n", inputFileName);
      exit(EXIT_FAILURE);
   }

   /* break loop with ^Z or ^D */
   while (TRUE) {
      char token, type[MAX_INPUT_SIZE];
      char name[MAX_INPUT_SIZE];
      char *end = fgets(line, sizeof(line)/sizeof(char), inputFile);
      int numTokens = sscanf(line, "%c %s %s", &token, name, type);

      if(pthread_mutex_lock(&mutex)){
         fprintf(stderr, "Error: Producer thread lock failed\n");
         exit(EXIT_FAILURE);
      }

      while(!(existingCommands < MAX_COMMANDS)){
         if(pthread_cond_wait(&podeProd, &mutex)){
            fprintf(stderr, "Error: Producer thread waiting failed\n");
            exit(EXIT_FAILURE);
         }
      } 

      if(end == NULL || *end == EOF){
         insertCommand("quit");
         if(pthread_mutex_unlock(&mutex)){
            fprintf(stderr, "Error: Producer thread unlock failed\n");
            exit(EXIT_FAILURE);
         }
         return NULL;
      }
      
      switch (token) {
         case 'c':
            if(numTokens != 3){
               fclose(inputFile);
               errorParse();
            }
            if(insertCommand(line))
               break;

         case 'l':
            if(numTokens != 2){
               fclose(inputFile);
               errorParse();
            }
            if(insertCommand(line))
               break;

         case 'm':
            if(numTokens != 3){
               fclose(inputFile);
               errorParse();
            }
            if(insertCommand(line))
               break;

         case 'd':
            if(numTokens != 2){
               fclose(inputFile);
               errorParse();
            }
            if(insertCommand(line))
               break;

         case '#':
            break;

         default: { /* error */
            fclose(inputFile);
            errorParse();
         }
      }

      if(pthread_cond_signal(&podeCons) || pthread_mutex_unlock(&mutex)){
         fprintf(stderr, "Error: Producer thread unlocking/signal failed\n");
         exit(EXIT_FAILURE);
      }
   }

   fclose(inputFile);
   return NULL;
}

void *applyCommands(void*ptr){
   lockArray *threadLocks = (lockArray *) ptr;
   while (TRUE){

      if(pthread_mutex_lock(&mutex)){
         fprintf(stderr, "Error: Consumer thread lock failed\n");
         exit(EXIT_FAILURE);
      }

      while(!(existingCommands > 0)){
         if(pthread_cond_wait(&podeCons, &mutex)){
            fprintf(stderr, "Error: Consumer thread wait failed\n");
            exit(EXIT_FAILURE);
         }
      } 

      const char* command = removeCommand();
      

      if(pthread_cond_signal(&podeProd) || pthread_mutex_unlock(&mutex)){
         fprintf(stderr, "Error: Consumer thread unlocking/signal failed\n");
         exit(EXIT_FAILURE);
      }
      
      if(strcmp(command, "quit") == 0){
         if(pthread_mutex_unlock(&mutex)){
            fprintf(stderr, "Error: Consumer thread lock failed\n");
            exit(EXIT_FAILURE);
         }
         return NULL;
      }

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
                  create(name, T_FILE, threadLocks);
                  break;
               case 'd':
                  fprintf(stdout,"Create directory: %s\n", name);
                  create(name, T_DIRECTORY, threadLocks);
                  break;
               default:
                  fprintf(stderr, "Error: invalid node type\n");
                  exit(EXIT_FAILURE);
            }
            break;

         case 'l':
            searchResult = lookup(name, threadLocks);
            if (searchResult >= 0)
               fprintf(stdout, "Search: %s found\n", name);
            else
               fprintf(stdout, "Search: %s not found\n", name);
            break;

         case 'd':
            fprintf(stdout, "Delete: %s\n", name);
            delete(name, threadLocks);
            break;

         case 'm':
            fprintf(stdout, "Move: %s to %s\n", name, type);   
            move(name, type, threadLocks);
            break;

         case 'q':
            return NULL;

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
   if(argc != 4)
      fprintf(stderr, "Error: Incorrect number of arguments given.\n");
   else if(!argv[1])
      fprintf(stderr, "Error: Input file given is NULL.\n");
   else if(!argv[2])
      fprintf(stderr, "Error: Output file given is NULL.\n");
   else if(atoi(argv[3]) <= 0)
      fprintf(stderr, "Error: Invalid number of threads given.\n");
   else {
      numberThreads = atoi(argv[3]);  //sets global var for the number of threads
      return;
   }
   exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
   argParse(argc, argv);

   struct timeval startTime, endTime;
   int i;

   pthread_t threadPool[numberThreads];
   pthread_t threadProd;

   /* init filesystem */
   init_fs();

   gettimeofday(&startTime, NULL);

   if(pthread_cond_init(&podeCons, NULL) || pthread_cond_init(&podeProd, NULL)) {
      fprintf(stderr, "Error: thread condition init failed.\n");
      exit(EXIT_FAILURE);
   }

   if(pthread_create(&threadProd, NULL, processInput, argv[1])){
      fprintf(stderr, "Error: thread creation failed.\n");
      exit(EXIT_FAILURE);
   }

   lockArray locks[numberThreads];

   for(i = 0; i < numberThreads; i++){
      locks[i].lockCount = 0;
      if(pthread_create(&threadPool[i], NULL, applyCommands, &locks[i])){
         fprintf(stderr, "Error: thread creation failed.\n");
         exit(EXIT_FAILURE);
      }
   }

   if(pthread_join(threadProd, NULL)){
      fprintf(stderr, "Error: thread merging failed.\n");
      exit(EXIT_FAILURE);
   }
   for(i = 0; i < numberThreads; i++){     
      if(pthread_join(threadPool[i], NULL)){
         fprintf(stderr, "Error: thread merging failed.\n");
         exit(EXIT_FAILURE);
      }
   }
   
   gettimeofday(&endTime, NULL);

   print_tecnicofs_tree(argv[2], startTime, endTime);
   
   /* release allocated memory */
   destroy_fs();
   
   exit(EXIT_SUCCESS);
}
