#ifndef SERVER_H
#define SERVER_H

#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 

#define MAX 80 
#define PORT 8080                   // Unused port
#define SA struct sockaddr

void comm(int confd);
void server_driver();

#endif  // SERVER_H