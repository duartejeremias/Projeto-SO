#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

int sockfd;
char server[MAX_FILE_NAME];
char client[MAX_FILE_NAME];
socklen_t servlen;
struct sockaddr_un serv_addr;

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

int tfsCreate(char *filename, char nodeType) {
  int command_len;
  char command[MAX_INPUT_SIZE], response[MAX_INPUT_SIZE];
  command_len = sprintf(command, "c %s %c", filename, nodeType);

  // processo de envio da mensagem do client para o server
  if (sendto(sockfd, command, command_len+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, response, sizeof(response)-1, 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  if(strcmp(response, "SUCCESS") == 0) return SUCCESS;
  else return FAIL;
}

int tfsDelete(char *path) {
  int command_len;
  char command[MAX_INPUT_SIZE], response[MAX_INPUT_SIZE];
  command_len = sprintf(command, "d %s", path);

  // processo de envio da mensagem do client para o server
  if (sendto(sockfd, command, command_len+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, response, sizeof(response)-1, 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  if(strcmp(response, "SUCCESS") == 0) return SUCCESS;
  else return FAIL;
}

int tfsMove(char *from, char *to) {
  int command_len;
  char command[MAX_INPUT_SIZE], response[MAX_INPUT_SIZE];
  command_len = sprintf(command, "m %s %s", from, to);

  // processo de envio da mensagem do client para o server
  if (sendto(sockfd, command, command_len+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, response, sizeof(response)-1, 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  if(strcmp(response, "SUCCESS") == 0) return SUCCESS;
  else return FAIL;
}

int tfsLookup(char *path) {
  int command_len;
  char command[MAX_INPUT_SIZE], response[MAX_INPUT_SIZE];
  command_len = sprintf(command, "l %s", path);

  // processo de envio da mensagem do client para o server
  if (sendto(sockfd, command, command_len+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, &response, sizeof(response)-1, 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  if(strcmp(response, "SUCCESS") == 0) return SUCCESS;
  else return FAIL;
}

int tfsPrint(char *fileName) {
  int command_len;
  char command[MAX_INPUT_SIZE], response[MAX_INPUT_SIZE];
  command_len = sprintf(command, "p %s", fileName);

  // processo de envio da mensagem do client para o server
  if (sendto(sockfd, command, command_len+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  } 

  if (recvfrom(sockfd, &response, sizeof(response)-1, 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  if(strcmp(response, "SUCCESS") == 0) return SUCCESS;
  else return FAIL;
}

int tfsMount(char * serverName) {
  socklen_t clilen;
  struct sockaddr_un client_addr;

  // criacao da socket
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0) ) < 0) {
    perror("client: can't open socket");
    return FAIL;
  }

  // creates client name
  sprintf(client, "/tmp/client_%d", getpid());

  unlink(client);

  clilen = setSockAddrUn(client, &client_addr);
  if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
    perror("client: bind error");
    return FAIL;
  }  

  // inicializacao dos atributos do server 
  strcpy(server, serverName);
  servlen = setSockAddrUn(server, &serv_addr);

  return SUCCESS;
}

int tfsUnmount() {
  if(close(sockfd) < 0){
    fprintf(stderr, "Error: Could not close socket\n");
    return FAIL;
  }
  
  if(unlink(client) < 0){
    fprintf(stderr, "Error: Coud not unlink\n");
    return FAIL;
  }

  return SUCCESS;
}
