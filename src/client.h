#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h> 
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> 
#include <sys/socket.h>
#include <unistd.h> 

#define MAX 80 
#define PORT 8080                   // Unused port
#define SA struct sockaddr

void client_comm(int sockfd);
void client_driver();

#endif  // CLIENT_H