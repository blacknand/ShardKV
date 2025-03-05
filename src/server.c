#include "server.h"


// In charge of the communcation between a server and a client
void comm(int confd) {
    char buff[MAX];
    int i;

    for (;;) {
        bzero(buff, MAX);
        read(confd, buff, sizeof(buff));        // Read message from client and read into buffer
        printf("[INFO] Recieved from client: %s\t", buff);
        bzero(buff, MAX);
        i = 0;

        // Write server message to client buffer
        while ((buff[i++] = getchar()) != '\n') {
            write(confd, buff, sizeof(buff));
            if (strncmp("[INFO] exit", buff, 4) == 0) {
                prinf("[INFO] Server exit\n");
                break;
            }
        }
    }
}


void server_driver() {
    int sockfd, confd, len;
    struct sockaddr_in server_addr, cli;

    // Create socket and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("[ERROR] TCP socket creation failed\n");
        exit(0);
    } else {
        printf("[INFO] TCP socket created successfully\n");
        bzero(&server_addr, sizeof(server_addr));
    }

    // Assign the IP address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind socket to IP address
    if ((bind(sockfd, (SA*)&server_addr, sizeof(server_addr))) != 0) {
        printf("[ERROR] TCP socket bind failed\n");
        exit(0);
    } else
        printf("[INFO] TCP socket successfully binded\n");

    // Start server listening
    if ((listen(sockfd, 5)) != 0) {
        printf("[ERROR] TCP server listening failed\n");
        exit(0);
    } else 
        printf("[INFO] TCP server listening\n");

    len = sizeof(cli);

    // Accept the data packet from client
    confd = accept(sockfd, (SA*)&cli, &len);

    if (confd < 0) {
        printf("[ERROR] TCP server accept failed\n");
        exit(0);
    } else
        printf("[INFO] TCP server accepted the client\n");

    comm(confd);            
    close(sockfd);          // Close socket
}