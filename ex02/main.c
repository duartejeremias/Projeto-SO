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
         return NULL;
      }
      existingCommands--;
      headQueue++;
      if(headQueue == MAX_COMMANDS) headQueue = 0;

      return inputCommands[i];
   }
   return NULL;
}

void errorParse(char *errorMessage){
   fprintf(stderr, "%s\n", errorMessage);
   exit(EXIT_FAILURE);
}

void *processInput(void *ptr){
   char *inputFileName = (char*) ptr;
   char line[MAX_INPUT_SIZE];
   FILE *inputFile = fopen(inputFileName, "r");

   if(inputFile == NULL) 
      errorParse("Error: Input file does not exist");

   char token, type[MAX_INPUT_SIZE];
   char name[MAX_INPUT_SIZE];


   while (TRUE) {
      
      char *end = fgets(line, sizeof(line)/sizeof(char), inputFile);
      int numTokens = sscanf(line, "%c %s %s", &token, name, type);

      if(pthread_mutex_lock(&mutex)){
         fclose(inputFile);
         errorParse("Error: Producer thread lock failed");
      }

      while(!(existingCommands < MAX_COMMANDS))
         if(pthread_cond_wait(&podeProd, &mutex)) {
            fclose(inputFile);
            errorParse("Error: Producer thread waiting failed");
         }
      

      if(end == NULL || *end == EOF){
         insertCommand("quit");
         if(pthread_cond_signal(&podeCons) || pthread_mutex_unlock(&mutex)) { 
            fclose(inputFile);
            errorParse("Error: Producer thread unlock/signal failed");
         }
         break;
      }
      
      switch (token) {
         case 'c':
            if(numTokens != 3){
               fclose(inputFile);
               errorParse("Error: command invalid");
            }
            if(insertCommand(line))
               break;

         case 'l':
            if(numTokens != 2){
               fclose(inputFile);
               errorParse("Error: command invalid");
            }
            if(insertCommand(line))
               break;

         case 'm':
            if(numTokens != 3){
               fclose(inputFile);
               errorParse("Error: command invalid");
            }
            if(insertCommand(line))
               break;

         case 'd':
            if(numTokens != 2){
               fclose(inputFile);
               errorParse("Error: command invalid");
            }
            if(insertCommand(line))
               break;

         case '#':
            break;

         default: { /* error */
            fclose(inputFile);
            errorParse("Error: command invalid");
         }
      }

      if(pthread_cond_signal(&podeCons) || pthread_mutex_unlock(&mutex)) {
         fclose(inputFile);
         errorParse("Error: Producer thread unlocking/signal failed");
      }
   }

   fclose(inputFile);
   return NULL;
}

void *applyCommands(void*ptr){
   lockArray *threadLocks = (lockArray *) ptr;

   char token, nodeType;
   char name[MAX_INPUT_SIZE], endName[MAX_INPUT_SIZE];
   
   while (TRUE){

      if(pthread_mutex_lock(&mutex))
         errorParse("Error: Consumer thread lock failed");

      while(!(existingCommands > 0)){
         if(pthread_cond_wait(&podeCons, &mutex))
            errorParse("Error: Consumer thread wait failed");
      } 

      const char* command = removeCommand();

      int numTokens;

      if(command == NULL){
         numTokens = 2;
         token = 'q';
      }
      else if(command[0] == 'c')
         numTokens = sscanf(command, "%c %s %c", &token, name, &nodeType);
      else if(command[0] == 'm')
         numTokens = sscanf(command, "%c %s %s", &token, name, endName);
      else 
         numTokens = sscanf(command, "%c %s", &token, name);

      if (numTokens < 2) 
         errorParse("Error: invalid command in Queue");

      if(pthread_cond_signal(&podeProd) || pthread_mutex_unlock(&mutex))
         errorParse("Error: Consumer thread unlocking/signal failed");

      int searchResult;
      switch (token) {
         case 'c':
            switch (nodeType) {
               case 'f':
                  fprintf(stdout, "Create file: %s\n", name);
                  create(name, T_FILE, threadLocks);
                  break;
               case 'd':
                  fprintf(stdout,"Create directory: %s\n", name);
                  create(name, T_DIRECTORY, threadLocks);
                  break;
               default:
                  errorParse("Error: invalid node type");
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
            fprintf(stdout, "Move: %s to %s\n", name, endName);   
            move(name, endName, threadLocks);
            break;

         case 'q':
            return NULL;

         default: { /* error */
            errorParse("Error: command to apply");
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
      errorParse("Error: Incorrect number of arguments given");
   else if(!argv[1])
      errorParse("Error: Input file given is NULL");
   else if(!argv[2])
      errorParse("Error: Output file given is NULL");
   else if(atoi(argv[3]) <= 0)
      errorParse("Error: Invalid number of threads given");
   else {
      numberThreads = atoi(argv[3]);  //sets global var for the number of threads
      return;
   }
}

/*
 * Initializes the pthread conds
 */
void init_cond(){
   if(pthread_cond_init(&podeCons, NULL) || pthread_cond_init(&podeProd, NULL)) 
      errorParse("Error: thread cond init failed");
}

/*
 * Terminates the pthread conds
 */
void destroy_cond(){
   if(pthread_cond_destroy(&podeCons) || pthread_cond_destroy(&podeProd))
      errorParse("Error: thread cond destruction failed");
}

int main(int argc, char* argv[]) {
   argParse(argc, argv);

   struct timeval startTime, endTime;
   int i;

   pthread_t threadPool[numberThreads+1];

   /* init filesystem */
   init_fs();

   gettimeofday(&startTime, NULL);

   init_cond();

   if(pthread_create(&threadPool[numberThreads], NULL, processInput, argv[1]))
      errorParse("Error: producer thread creation failed");

   lockArray locks[numberThreads];

   for(i = 0; i < numberThreads; i++){
      locks[i].lockCount = 0;
      if(pthread_create(&threadPool[i], NULL, applyCommands, &locks[i]))
         errorParse("Error: consumer thread creation failed");
   }

   for(i = 0; i <= numberThreads; i++){     
      if(pthread_join(threadPool[i], NULL))
         errorParse("Error: thread merging failed");
   }
   
   gettimeofday(&endTime, NULL);

   print_tecnicofs_tree(argv[2]);

   printf("TecnicoFS completed in %.4f seconds.\n", (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6);
   
   destroy_cond();

   /* release allocated memory */
   destroy_fs();
   
   exit(EXIT_SUCCESS);
}
