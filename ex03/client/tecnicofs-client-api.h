#ifndef API_H
#define API_H

#include "../tecnicofs-api-constants.h"
#include <sys/socket.h>
#include <sys/un.h>

#define SUCCESS 0
#define FAIL -1

int setSockAddrUn(char *path, struct sockaddr_un *addr);
int tfsCreate(char *path, char nodeType);
int tfsDelete(char *path);
int tfsLookup(char *path);
int tfsMove(char *from, char *to);
int tfsPrint(char *fileName);
int tfsMount(char* serverName);
int tfsUnmount();

#endif /* CLIENT_H */
