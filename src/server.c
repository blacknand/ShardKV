/**
 * To see the TCP server API please see ~/docs/api_usage.md
 * 
 * The TCP server will handle client requests for operations such as PUT, GET and DELETE on the key-value pairs
 * 
 * The TCP server is currently IPv4, IPv6 will most likely be added in the future
 */

#include "server.h"


// In charge of the communcation between a server and a client
void comm(int confd) {
    char buff[MAX];
    int i;
    char command[10], key[50], value[100];

    // Infinite loop to keep server alive
    for (;;) {
        bzero(buff, MAX);
        read(confd, buff, sizeof(buff));        // Read message from client and read into buffer
        printf("[INFO] Recieved from client: %s\t", buff);

        sscanf(buff, "%s %s %[^\n]", command, key, value);      // Read the commands

        // Check for the commands
        if (strcmp(command, "PUT") == 0) {
            put(key, value);
            write(confd, "OK\n", 3);
        } else if (strcmp(command, "GET") == 0) {
            char *result = get(key);
            if (result)
                write(confd, result, strlen(result));
            else
                write(confd, "[ERROR] key not found\n", 20);
        } else if (strcmp(command, "DELETE") == 0) {
            delete(key);
            write(confd, "OK\n", 3);
        } else if (strcmp(command, "exit") == 0) {
                prinf("[INFO] Server exit\n");
                break;
        } else
            write(confd, "[ERROR] invalid command\n", 22);
    }
}


void server_driver() {
    int sockfd, confd, len;
    struct sockaddr_in server_addr, cli;

    // 1. Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("[ERROR] TCP socket creation failed\n");
        exit(0);
    } else {
        printf("[INFO] TCP socket created successfully\n");
        bzero(&server_addr, sizeof(server_addr));
    }

    // 2. Assign the IP address and port
    server_addr.sin_family = AF_INET;                           // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);            // Accept connections on any network inteface
    server_addr.sin_port = htons(PORT);                         // Convert port to network byte order

    // 3. Bind socket to IP address
    if ((bind(sockfd, (SA*)&server_addr, sizeof(server_addr))) != 0) {
        printf("[ERROR] TCP socket bind failed\n");
        exit(0);
    } else
        printf("[INFO] TCP socket successfully binded\n");

    // 4. Start server listening
    if ((listen(sockfd, 5)) != 0) {
        printf("[ERROR] TCP server listening failed\n");
        exit(0);
    } else 
        printf("[INFO] TCP server listening\n");

    len = sizeof(cli);

    // 5. Accept the data packet from client
    confd = accept(sockfd, (SA*)&cli, &len);

    if (confd < 0) {
        printf("[ERROR] TCP server accept failed\n");
        exit(0);
    } else
        printf("[INFO] TCP server accepted the client\n");

    comm(confd);            // 6. Handle client communication
    close(sockfd);          // 7. Close socket
}