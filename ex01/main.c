#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include "fs/operations.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

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
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

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

void applyCommands(){
    while (numberCommands > 0){
        const char* command = removeCommand();
        if (command == NULL){
            continue;
        }

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
                switch (type) {
                    case 'f':
                        fprintf(stdout, "Create file: %s\n", name);
                        create(name, T_FILE);
                        break;
                    case 'd':
                        fprintf(stdout,"Create directory: %s\n", name);
                        create(name, T_DIRECTORY);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                searchResult = lookup(name);
                if (searchResult >= 0)
                    fprintf(stdout, "Search: %s found\n", name);
                else
                    fprintf(stdout, "Search: %s not found\n", name);
                break;
            case 'd':
                fprintf(stdout, "Delete: %s\n", name);
                delete(name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*
 * Validates command line arguments
 * Input: argument count and arguments
 * Output: 1 if invalid, 0 if valid
 */
int invalid_arg(int argc, char* argv[]){
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
   else return 0;
   return 1;
}

int main(int argc, char* argv[]) {
   clock_t endTime, startTime = clock();
   float timeSpent;

   if(invalid_arg(argc, argv)) exit(EXIT_FAILURE);

   /* init filesystem */
   init_fs();

   /* process input and print tree */
   processInput(argv[1]);
   applyCommands();

   print_tecnicofs_tree(argv[2]);

   /* release allocated memory */
   destroy_fs();

   endTime = clock();
   timeSpent = (float)(endTime - startTime) / CLOCKS_PER_SEC;
   printf("TecnicoFS completed in %.4f seconds.\n", timeSpent);

   exit(EXIT_SUCCESS);
}
