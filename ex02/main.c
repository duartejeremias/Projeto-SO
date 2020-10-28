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

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100
#define TRUE 1
#define FALSE 0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int numberThreads = 0;

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

void *applyCommands(void*ptr){
   lockArray *threadLocks = (lockArray *) ptr;
   while (TRUE){
      pthread_mutex_lock(&mutex);
      if(numberCommands == 0){ //if command count is 0, thread should end
         pthread_mutex_unlock(&mutex);
         break;
      }
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      const char* command = removeCommand();

      if (command == NULL){ //if command is NULL, thread should end
         pthread_mutex_unlock(&mutex);
         break;
      }
      pthread_mutex_unlock(&mutex);

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
            unlock(threadLocks);
            break;

         case 'd':
            fprintf(stdout, "Delete: %s\n", name);
            delete(name, threadLocks);
            break;

         case 'm':
            fprintf(stdout, "Move: %s to %s\n", name, type);   
            move(name, type, threadLocks);
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
   gettimeofday(&startTime, NULL);

   /* init filesystem */
   init_fs();

   /* process input and print tree */
   processInput(argv[1]);

   lockArray locks[numberThreads];

   for(i = 0; i < numberThreads; i++){
      locks[i].lockCount = 0;
      if(pthread_create(&threadPool[i], NULL, applyCommands, &locks[i])){
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
   

   print_tecnicofs_tree(argv[2]);

   gettimeofday(&endTime, NULL);

   /* release allocated memory */
   destroy_fs();

   printf("TecnicoFS completed in %.4f seconds.\n", (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1e6);

   exit(EXIT_SUCCESS);
}
