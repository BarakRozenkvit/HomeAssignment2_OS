/*
        TCP/IP client
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 4050
#define SERVER_IP_ADDRESS "127.0.0.1"

#define BUFFER_SIZE 16

int main(int argc,char* argv[]) {
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

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        printf("Could not create socket : %d", errno);
        return -1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    int rval = inet_pton(AF_INET, (const char *)address, &serverAddress.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    printf("Client is Waiting to connect to Server %s with port %d\n",address,port);
    int connectResult = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1) {
        printf("connect() failed with error code : %d", errno);
        close(sock);
        return -1;
    }

    printf("Client is Connected to Server\n");

    while (1) {
        if (c == 'o' || c == 'b') {
            char bufferReply[BUFFER_SIZE] = {'\0'};
            int bytesReceived = recv(sock, bufferReply, BUFFER_SIZE, 0);
            printf("%s",bufferReply);
            if(bytesReceived > 2){
                break;
            }
        }

        if (c == 'i' || c == 'b') {
            char message[1];
            scanf("%s", message);
            int messageLen = strlen(message) + 1;
            int bytesSent = send(sock, message, messageLen, 0);
            if( bytesSent <=0){
                break;
            }
        }
    }
    close(sock);
    return 0;
}

