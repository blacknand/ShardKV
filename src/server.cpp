/**
 * To see the TCP server API please see ~/docs/api_usage.md
 * 
 * The TCP server will handle client requests for operations such as store.put, store.get and store.delete on the key-value pairs
 * 
 * The TCP server is currently IPv4, IPv6 will most likely be added in the future
 */

#include "server.h"
#include "kv_store.h"


// In charge of the communcation between a server and a client
void comm(int confd, int server_sock) {
    char buff[MAX];
    char command[10], key[50], value[100];
    KVStore store;

    // Infinite loop to keep server alive
    for (;;) {
        bzero(buff, MAX);
        int bytes_read = read(confd, buff, sizeof(buff) - 1);        // Read message from client and read into buffer
        
        if (bytes_read <= 0) {
            printf("[INFO] Recieved from client: %s\n", buff);
            break;
        }

        buff[bytes_read] = '\0';
        printf("[INFO] recieved from TCP client: %s\n", buff);

        // Read the commands
        int command_val = sscanf(buff, "%s %s %[^\n]", command, key, value);

        if (strcmp(command, "exit") == 0)  {
            printf("[INFO] Server exit\n");
            write(confd, "exit\n", 5);
            close(confd);
            return;
        }

        if (command_val < 2) {
            write(confd, "[ERROR] Invalid command format\n", 30);
            continue; 
        }

        // Check for the commands
        if (strcmp(command, "PUT") == 0) {
            store.put(key, value);
            write(confd, "OK\n", 3);
        } else if (strcmp(command, "GET") == 0) {
            std::string result = store.get(key);
            if (!result.empty())
                write(confd, result.c_str(), result.length());
            else
                write(confd, "[ERROR] key not found\n", 20);
        } else if (strcmp(command, "DELETE") == 0) {
            int result = store.remove(key);
            if (result == 0)
                write(confd, "OK\n", 3);
            else
                write(confd, "[ERROR] key not found\n", 20);
        } else
            write(confd, "[ERROR] invalid command\n", 22);
    }
}


void server_driver() {
    int sockfd, confd;
    socklen_t len;
    struct sockaddr_in server_addr, cli;

    // TODO: concurrency to hanlde multiple TCP sockets at the same time

    // 1. Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("[ERROR] TCP socket creation failed\n");
        exit(0);
    } else {
        printf("[INFO] TCP socket created successfully\n");
        bzero(&server_addr, sizeof(server_addr));
    }

    // Allow address to reuse so we can restart server quickly after connection
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&server_addr, sizeof(server_addr));

    // 2. Assign the IP address and port
    server_addr.sin_family = AF_INET;                           // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);            // Accept connections on any network inteface
    server_addr.sin_port = htons(PORT);                         // Convert port to network byte order

    // 3. Bind socket to IP address
    if ((bind(sockfd, (SA*)&server_addr, sizeof(server_addr))) != 0) {
        printf("[ERROR] TCP socket bind failed\n");
        close(sockfd);
        exit(0);
    } else
        printf("[INFO] TCP socket successfully binded\n");

    // 4. Start server listening
    if ((listen(sockfd, 5)) != 0) {
        printf("[ERROR] TCP server listening failed\n");
        close(sockfd);
        exit(0);
    } else 
        printf("[INFO] TCP server listening\n");

    len = sizeof(cli);
    
    // 5. Accept the data packet from client
    confd = accept(sockfd, (SA*)&cli, &len);

    if (confd < 0) {
        printf("[ERROR] TCP server accept failed\n");
        close(sockfd);
        exit(0);
    } else 
        printf("[INFO] TCP server accepted the client\n");

    comm(confd, sockfd);            // 6. Handle client communication

    close(sockfd);          // 7. Close socket
    printf("[INFO] server has shutdown\n");
}