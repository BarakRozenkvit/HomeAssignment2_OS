/*
    TCP/IP-server
*/

#include <stdio.h>

// Linux and other UNIXes
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 4455  // The port that the server listens
#define BUFFER_SIZE 32

int main(int argc,char* argv[]) {
    int port = SERVER_PORT;
    char c;
    if (argc > 2){
        c = argv[1][0];
        port = atoi(argv[2]);}

    switch (c) {
        case 'i':
            printf("Server functions as Input\n");
            break;
        case 'o':
            printf("Server functions as Output\n");
            break;
        case 'b':
            printf("Server functions as I/O\n");
            break;

    }
    printf("Welcome to Game Localhost Server %d\n",port);
    printf("------------------------------------------\n");

    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 0 means default protocol for stream sockets (Equivalently, IPPROTO_TCP)
    if (listeningSocket == -1) {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }

    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0) {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        printf("Bind failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }

    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1) {
        printf("listen() failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }

    printf("Waiting for incoming TCP-connections...\n");
    struct sockaddr_in clientAddress;  //
    socklen_t clientAddressLen = sizeof(clientAddress);

    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (clientSocket == -1) {
        printf("listen failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }
    printf("A Client Connected\n");

    while (1) {

        if (c == 'o' || c == 'b') {
            char bufferReply[BUFFER_SIZE] = {'\0'};
            int bytesReceived = recv(clientSocket, bufferReply, BUFFER_SIZE, 0);
            printf("%s", bufferReply);
            if (bytesReceived > 2) {
                break;
            }
        }

        if (c == 'i' || c == 'b') {
            char message[1];
            scanf("%s", message);
            int messageLen = strlen(message) + 1;
            int bytesSent = send(clientSocket, message, messageLen, 0);
            if (bytesSent <= 0) {
                break;
            }
        }
    }

    close(clientSocket);
    close(listeningSocket);
    return 0;
}
