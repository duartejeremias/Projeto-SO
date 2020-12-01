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

#define SUCCESS_MSG "SUCCESS"
#define FAIL_MSG "FAIL"

int numberThreads = 0;
int sockfd;

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

   socklen_t addrlen;
   struct sockaddr_un client_addr;
   char command[MAX_INPUT_SIZE];
   int c, result;

   addrlen = sizeof(struct sockaddr_un);
   
   while (TRUE){
      // recebe mensagem do client
      c = recvfrom(sockfd, command, sizeof(command)-1, 0, (struct sockaddr *)&client_addr, &addrlen);

      if(c  <= 0) continue;

      int numTokens;

      command[c]='\0'; // caso preventivo

      if(command[0] == 'c')
         numTokens = sscanf(command, "%c %s %c", &token, name, &nodeType);
      else if(command[0] == 'm')
         numTokens = sscanf(command, "%c %s %s", &token, name, endName);
      else 
         numTokens = sscanf(command, "%c %s", &token, name);

      if (numTokens < 2) 
         errorParse("Error: invalid command in Queue");

      switch (token) {
         case 'c':
            switch (nodeType) {
               case 'f':
                  fprintf(stdout, "Create file: %s\n", name);
                  result = create(name, T_FILE, threadLocks);
                  break;
               case 'd':
                  fprintf(stdout,"Create directory: %s\n", name);
                  result = create(name, T_DIRECTORY, threadLocks);
                  break;
               default:
                  errorParse("Error: invalid node type");
            }
            break;

         case 'l':
            result = lookup(name, threadLocks);
            if (result >= 0) {
               fprintf(stdout, "Search: %s found\n", name);
               result = SUCCESS;
            } else {
               fprintf(stdout, "Search: %s not found\n", name);
               result = FAIL;
            }
            break;

         case 'd':
            fprintf(stdout, "Delete: %s\n", name);
            result = delete(name, threadLocks);
            break;

         case 'm':
            fprintf(stdout, "Move: %s to %s\n", name, endName);   
            result = move(name, endName, threadLocks);
            break;

         case 'p':
            fprintf(stdout, "Print file system: %s\n", name);
            result = print_tecnicofs_tree(name, threadLocks);
            break;

         default: { /* error */
            errorParse("Error: command to apply");
         }
      }
      if(result == SUCCESS) sendto(sockfd, SUCCESS_MSG, sizeof(SUCCESS_MSG), 0, (struct sockaddr *)&client_addr, addrlen);
      else sendto(sockfd, FAIL_MSG, sizeof(FAIL_MSG), 0, (struct sockaddr *)&client_addr, addrlen);
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

   struct sockaddr_un server_addr;
   socklen_t addrlen;

   lockArray locks[numberThreads];
   pthread_t threadPool[numberThreads];

   /* initializes file system */
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

   for(int i = 0; i < numberThreads; i++){
      locks[i].lockCount = 0;
      if(pthread_create(&threadPool[i], NULL, applyCommands, &locks[i]))
         errorParse("Error: thread creation failed");
   }

   for(int i = 0; i < numberThreads; i++){     
      if(pthread_join(threadPool[i], NULL))
         errorParse("Error: thread merging failed");
   }

   /* release allocated memory */
   destroy_fs();
   
   close(sockfd);
   unlink(argv[2]);

   exit(EXIT_SUCCESS);
}
