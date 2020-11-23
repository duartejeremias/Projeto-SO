/* 
 * Terceiro exercicio do projeto de SO
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
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include "fs/operations.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100
#define TRUE 1
#define FALSE 0

#define INDIM 30
#define OUTDIM 512

int numberThreads = 0;

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

   if (addr == NULL)
      return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

void errorParse(char *errorMessage){
   fprintf(stderr, "%s\n", errorMessage);
   exit(EXIT_FAILURE);
}

void *applyCommands(void*ptr){
   lockArray *threadLocks = (lockArray *) ptr;

   char token, nodeType;
   char name[MAX_INPUT_SIZE], endName[MAX_INPUT_SIZE];
   
   while (TRUE){

      const char* command = "wack";

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

         case 'p':
            fprintf(stdout, "Print file system\n");
            print_tecnicofs_tree(name);
            break;

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
   if(argc != 3)
      errorParse("Error: Incorrect number of arguments given");
   
   else if(atoi(argv[1]) <= 0)
      errorParse("Error: Invalid number of threads given");

   else if(!argv[2])
      errorParse("Error: Socket name given is NULL");

   else {
      numberThreads = atoi(argv[1]);  //sets global var for the number of threads
      return;
   }
}

int main(int argc, char* argv[]) {
   argParse(argc, argv);

   int i, hasCreatedThreads = FALSE;

   int sockfd;
   struct sockaddr_un server_addr;
   socklen_t addrlen;

   lockArray locks[numberThreads];
   pthread_t threadPool[numberThreads];

   // initializes file system
   init_fs();

   /* init server */
   if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) 
      errorParse("server: can't open socket");

   unlink(argv[2]);
   
   addrlen = setSockAddrUn (argv[2], &server_addr);
   if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
      perror("server: bind error");
      exit(EXIT_FAILURE);
   }

   while(1){
      struct sockaddr_un client_addr;
      char in_buffer[INDIM], success[] = "SUCCESS", fail[] = "FAIL";
      int c;

      addrlen = sizeof(struct sockaddr_un);
      // recebe mensagem do client
      c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0, (struct sockaddr *)&client_addr, &addrlen);

      if (c <= 0) continue; // se nao foi enviada mensagem, passa a proxima iteracao

      sendto(sockfd, success, sizeof(success), 0, (struct sockaddr *)&client_addr, addrlen);

      //Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
      in_buffer[c]='\0';
      fprintf(stdout, "%s\n", in_buffer);
   }

   for(i = 0; i < numberThreads; i++){
      locks[i].lockCount = 0;
      if(pthread_create(&threadPool[i], NULL, applyCommands, &locks[i]))
         errorParse("Error: consumer thread creation failed");
   }


   /* release allocated memory */
   destroy_fs();
   
   close(sockfd);
   unlink(argv[2]);

   exit(EXIT_SUCCESS);
}
