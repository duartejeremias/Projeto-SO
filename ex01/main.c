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

void errorParse(FILE *outputFile){
    fprintf(outputFile, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(char *inputFileName, FILE *outputFile){
    char line[MAX_INPUT_SIZE];
    FILE *inputFile = fopen(inputFileName, "r");

    if(inputFile == NULL){
      fprintf(outputFile, "Error: file %s does not exist\n", inputFileName);
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
                    errorParse(outputFile);
                if(insertCommand(line))
                    break;
                return;

            case 'l':
                if(numTokens != 2)
                    errorParse(outputFile);
                if(insertCommand(line))
                    break;
                return;

            case 'd':
                if(numTokens != 2)
                    errorParse(outputFile);
                if(insertCommand(line))
                    break;
                return;

            case '#':
                break;

            default: { /* error */
                errorParse(outputFile);
            }
        }
    }
    fclose(inputFile);
}

void applyCommands(FILE *outputFile){
    while (numberCommands > 0){
        const char* command = removeCommand();
        if (command == NULL){
            continue;
        }

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(outputFile, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
                switch (type) {
                    case 'f':
                        fprintf(outputFile, "Create file: %s\n", name);
                        create(name, T_FILE, outputFile);
                        break;
                    case 'd':
                        fprintf(outputFile,"Create directory: %s\n", name);
                        create(name, T_DIRECTORY, outputFile);
                        break;
                    default:
                        fprintf(outputFile, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                searchResult = lookup(name);
                if (searchResult >= 0)
                    fprintf(outputFile, "Search: %s found\n", name);
                else
                    fprintf(outputFile, "Search: %s not found\n", name);
                break;
            case 'd':
                fprintf(outputFile, "Delete: %s\n", name);
                delete(name, outputFile);
                break;
            default: { /* error */
                fprintf(outputFile, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main(int argc, char* argv[]) {
   FILE *outputFile = fopen(argv[2], "w");
   clock_t endTime, startTime = clock();
   float timeSpent;

   /* init filesystem */
   init_fs();

   /* process input and print tree */
   processInput(argv[1], outputFile);
   applyCommands(outputFile);

   print_tecnicofs_tree(outputFile);
   fclose(outputFile);

   /* release allocated memory */
   destroy_fs();

   endTime = clock();
   timeSpent = (float)(endTime - startTime) / CLOCKS_PER_SEC;
   printf("TecnicoFS completed in %.4f seconds.\n", timeSpent);

   exit(EXIT_SUCCESS);
}
