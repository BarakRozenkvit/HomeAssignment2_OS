
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "stdio.h"

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 5060
#define BUFFER_SIZE 1024

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

    int serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == -1) {
        printf("Could not create socket : %d\n", errno);
        return -1;
    }

    // setup Server address structure
    struct sockaddr_in serverAddress;
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port); // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
    int ret = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &(serverAddress.sin_addr));
    if (ret <= 0) {
        printf("inet_pton() failed\n");
        return -1;
    }

    // Bind
    int bindResult = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        printf("bind() failed with error code : %d\n", errno);
        // cleanup the socket;
        close(serverSocket);
        return -1;
    }
    printf("After bind(). Waiting for clients\n");

    // setup Client address structure
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    memset((char *)&clientAddress, 0, sizeof(clientAddress));

    // keep listening for data
    while (1) {

        if (c == 'o' || c == 'b') {
            char buffer[BUFFER_SIZE] = {'\0'};
            int recv_len = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *) &clientAddress,
                                    &clientAddressLen);
            if (recv_len <= 0) {
                printf("recvfrom() failed with error code : %d", errno);
                close(serverSocket);
                break;
            }
            char clientIPAddrReadable[32] = {'\0'};
            inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPAddrReadable, sizeof(clientIPAddrReadable));
            printf("%s", buffer);
        }

        if (c == 'i' || c == 'b') {
            char message[1];
            scanf("%s", message);
            int messageLen = strlen(message) + 1;
            int sendResult = sendto(serverSocket, message, messageLen, 0, (struct sockaddr *) &clientAddress,
                                    clientAddressLen);
            if (sendResult <= 0) {
                printf("sendto() failed with error code : %d\n", errno);
                break;
            }
        }
    }
    close(serverSocket);
    return 0;
}
