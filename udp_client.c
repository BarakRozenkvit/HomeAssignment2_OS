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
#define BUFFER_SIZE 16

int main(int argc, char* argv[]) {
    int port = SERVER_PORT;
    char* address = SERVER_IP_ADDRESS;
    char c;

    if (argc > 3){
        c = argv[1][0];
        port = atoi(argv[2]);
        address=argv[3];}
    else{
        perror("arguments\n");
        return 1;
    }

    printf("Welcome to Game Client\n");
    switch (c) {
        case 'i':
            printf("Client functions as Input\n");
            break;
        case 'o':
            printf("Client functions as Output\n");
            break;
        case 'b':
            printf("Client functions as I/O\n");
            break;

    }
    printf("-------------------------------\n");

    int clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == -1) {
        printf("Could not create socket : %d", errno);
        return -1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    while(1) {
        if (c == 'o' || c == 'b') {
            char bufferReply[BUFFER_SIZE] = {'\0'};
            struct sockaddr_in fromAddress;
            socklen_t fromAddressSize = sizeof(fromAddress);
            memset((char *) &fromAddress, 0, sizeof(fromAddress));

            int recvLen = recvfrom(clientSocket, bufferReply, sizeof(bufferReply) - 1, 0,
                                   (struct sockaddr *) &fromAddress, &fromAddressSize);
            if (recvLen == -1) {
                printf("recvfrom() failed with error code  : %d", errno);
                return -1;
            }

            printf("[%s:%d] Data: %s", inet_ntoa(fromAddress.sin_addr), ntohs(fromAddress.sin_port), bufferReply);
        }

        if (c == 'i' || c == 'b') {
            char message[1];
            scanf("%s", message);
            int messageLen = strlen(message) + 1;
            // send the message
            int sendResult = sendto(clientSocket, message, messageLen, 0, (struct sockaddr *) &serverAddress,
                                    sizeof(serverAddress));
            if (sendResult == -1) {
                printf("sendto() failed with error code  : %d", errno);
                return -1;
            }
        }
    }

    close(clientSocket);

    return 0;
}
