#include "client.h"


void client_comm(int sockfd) {
    char buff[MAX];
    int n;

    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter command: ");
        n = 0;

        while ((buff[n++] = getchar()) != '\n') {
            write(sockfd, buff, sizeof(buff));
            bzero(buff, sizeof(buff));
            read(sockfd, buff, sizeof(buff));
            printf("[INFO] Recieved from TCP Server: %s", buff);

            if ((strncmp(buff, "exit", 4)) == 0) {
                printf("[INFO] TCP server exit");
                break;
            }
        }
    }
}


void client_driver() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("[ERROR] TCP socket creation failed\n");
        exit(0);
    }
    else
        printf("[INFO] TCP socket successfully created\n");

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");          // Pingback address
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("[ERROR] connection with the TCP server failed\n");
        exit(0);
    } else
        printf("[INFO] connected to the TCP server\n");

    client_comm(sockfd);
    close(sockfd);
}