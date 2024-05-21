#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>


#define BUFFER_SIZE 16
#define MAXCLIENTS 10

void tcpmux_server_socket(int port,int* fdsArr){
    int listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == -1) {
        perror("socket");
        exit(1);
    }

    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0) {
        perror("setsockopt() failed with error code");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        perror("Bind failed with error code");
        close(listeningSocket);
        exit(1);
    }

    int listenResult = listen(listeningSocket, MAXCLIENTS);
    if (listenResult == -1) {
        printf("listen() failed with error code");
        close(listeningSocket);
        exit(1);
    }

    // Create array for storing client sockets and events
    struct pollfd fds_poll[MAXCLIENTS];
    memset(fds_poll,0,sizeof(fds_poll));

    // Add listening socket for new connections
    fds_poll[0].fd = listeningSocket;
    fds_poll[0].events = POLLIN;

    int current_clients = 0;
    int running=1;

    printf("Server is Waiting for incoming TCP-connections on port: %d...\n",port);
    while(running){
        // Start poll with not timeout == -1
        int ret = poll(fds_poll,current_clients + 1,-1);
        if(ret == -1){
            perror("poll");
            break;
        }

        // Handle events
        for(int i=0;i<current_clients + 1 && running;i++){
            if(fds_poll[i].fd == -1){
                continue;
            }

            //Check for new connections
            if(fds_poll[i].fd == listeningSocket && (fds_poll[i].revents & POLLIN)){
                printf("Listening socket is changed\n");
                // Code for accept like tcp_server
                struct sockaddr_in clientAddress;  //
                socklen_t clientAddressLen = sizeof(clientAddress);
                memset(&clientAddress, 0, sizeof(clientAddress));
                clientAddressLen = sizeof(clientAddress);
                int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
                if (clientSocket == -1) {
                    printf("listen failed with error code");
                    close(listeningSocket);
                    exit(1);
                }

                if(current_clients == MAXCLIENTS){
                    printf("Max connections\n");
                    close(clientSocket);
                } else{
                    fds_poll[current_clients + 1].fd = clientSocket;
                    fds_poll[current_clients + 1].events = POLLIN;
                    current_clients++;
                    int pid_new_client = fork();
                    if(!pid_new_client){
                        fdsArr[0] = clientSocket;
                        fdsArr[1] = clientSocket;
                        running = 0;
                        break;
                    }
                    printf("Client %d is connected, new pid is %d\n",current_clients,pid_new_client);
                }

            }
        }
    }
}

void tcp_server_socket(int port, int* fdsArr){
    int listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == -1) {
        perror("socket");
        exit(1);
    }

    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0) {
        perror("setsockopt() failed with error code");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        perror("Bind failed with error code");
        close(listeningSocket);
        exit(1);
    }

    int listenResult = listen(listeningSocket, 0);
    if (listenResult == -1) {
        printf("listen() failed with error code");
        close(listeningSocket);
        exit(1);
    }

    printf("Server is Waiting for incoming TCP-connections on port: %d...\n",port);
    struct sockaddr_in clientAddress;  //
    socklen_t clientAddressLen = sizeof(clientAddress);

    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (clientSocket == -1) {
        printf("listen failed with error code");
        close(listeningSocket);
        exit(1);
    }

    fdsArr[0] = clientSocket;
    fdsArr[1] = listeningSocket;
}

void tcp_client_socket(int port, char* address, int* fdsArr){


    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1) {
        printf("Could not create socket");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    struct hostent *server;
    if ((server = gethostbyname(address)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    memcpy(&(serverAddress.sin_addr), server->h_addr_list[0], sizeof(serverAddress.sin_addr));


    printf("Client is waiting to connect on port: %d with server address: %s...\n",port,address);

    int connectResult = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1) {
        printf("connect() failed with error code");
        close(clientSocket);
        exit(1);
    }

    fdsArr[0] = clientSocket;
    fdsArr[1] = clientSocket;
}

void udp_server_socket(int port, int* fdsArr){

    int serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == -1) {
        printf("Could not create socket");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    int ret = inet_pton(AF_INET, (const char *)"127.0.0.1", &(serverAddress.sin_addr));
    if (ret <= 0) {
        printf("inet_pton() failed\n");
        exit(1);
    }

    int bindResult = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        printf("bind() failed with error code\n");
        close(serverSocket);
        exit(1);
    }
    printf("UDP Server is Binding on port: %d...\n",port);
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    memset((char *)&clientAddress, 0, sizeof(clientAddress));

    char buffer[BUFFER_SIZE];
    int recv_len = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *) &clientAddress,
                            &clientAddressLen);
    if (recv_len <= 0) {
        printf("recvfrom() failed with error code");
        close(serverSocket);
        exit(1);
    }
    char clientIPAddrReadable[32] = {'\0'};
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPAddrReadable, sizeof(clientIPAddrReadable));

    // Saves the client credetials for write
    int connectResult = connect(serverSocket, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
    if(connectResult < 0){
        printf("\n Error : Connect Failed \n");
        exit(1);
    }
    printf("Saved my Client Info!\n");

    fdsArr[0] = serverSocket;
    fdsArr[1] = serverSocket;
}

void udp_client_socket(int port, char* address, int* fdsArr){

    int clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == -1) {
        printf("Could not create socket");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    struct hostent *server;
    if ((server = gethostbyname(address)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    memcpy(&(serverAddress.sin_addr), server->h_addr_list[0], sizeof(serverAddress.sin_addr));

    char* message = "Hi";
    int messageLen = strlen(message) + 1;
    int sendResult = sendto(clientSocket, message, messageLen, 0, (struct sockaddr *) &serverAddress,
                            sizeof(serverAddress));
    if (sendResult <= 0) {
        printf("sendto() failed with error code");
    }

    // Saves the client credetials for write
    int connectResult = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if(connectResult < 0){
        printf("\n Error : Connect Failed \n");
        exit(1);
    }

    printf("Saved my Server Info!\n");

    fdsArr[0] = clientSocket;
    fdsArr[1] = clientSocket;
}

int argv_to_socket(char* str, int* fdsArr){
    int size = strlen(str);
    if(size < 7 || size > 20){
        perror("bad input");
        exit(1);
    }
    char protocol[4] = {str[0],str[1],str[2],'\0'};
    char type = str[3];

    if(type == 'S'){
        char* portStr[10];
        strcpy(portStr,str+4);
        int port = atoi(portStr);
        if (strcasecmp(protocol,"TCP")==0) {
            tcp_server_socket(port, fdsArr);
            return 0;
        }
        else if(strcasecmp(protocol,"UDP")==0){
            udp_server_socket(port,fdsArr);
            return 0;
        }
    }

    else if(type == 'C'){
        // TODO: make code prettier
        char* address;
        char* portStr;
        int sep;
        for(int i=0;i< strlen(str);i++){
            if(str[i] == ','){
                sep = i;
                break;
            }
        }
        address = (char *) malloc(sizeof(char)*sep-4);
        if(!address){
            perror("malloc");
            return 1;
        }
        portStr = (char *) malloc(sizeof(char)* strlen(str)-sep+1);
        if(!portStr){
            free(address);
            perror("malloc");
            return 1;
        }
        strncpy(address,str+4,sizeof(char)*sep-4);
        strcpy(portStr,str+sep+1);
        int port = atoi(portStr);
        if (strcasecmp(protocol,"TCP")==0) {
            tcp_client_socket(port,address,fdsArr);
            return 0;
        }
        else if(strcasecmp(protocol,"UDP")==0){
            udp_client_socket(port,address,fdsArr);
            return 0;
        }

        else {
            exit(1);
        }
    }

    if (type == 'M'){
        char* portStr[10];
        strcpy(portStr,str+7);
        int port = atoi(portStr);
        tcpmux_server_socket(port,fdsArr);
        return 0;
    }

    else {
        exit(1);
    }

    return 0;
}

int main(int argc,char* argv[]){
    printf("Main PID: %d\n",getpid());
    char process_path[256];
    getcwd(process_path,sizeof(process_path));
    char process_name[256] = {'/','\0'};
    char process_argv[256];
    char* token;
    int fds_input[2] = {0,0};
    int fds_output[2] = {1,1};
    int muteAlarm = 1;
    int e_is_declared=0;
    int timeout=0;
    int c=1;
    while (c != -1) {
        c = getopt (argc, argv, "t:b:o:i:e:");
        switch (c) {
            case 'e':
                e_is_declared = 1;
                token = strsep(&optarg," ");
                strcat(process_name,token);
                strncat(process_path,process_name,sizeof(process_path) - strlen(process_path) - 1);
                token = strsep(&optarg," ");
                strcat(process_argv,token);
                break;

            case 'i':
                printf("Input functions as %s\n",optarg);
                argv_to_socket(optarg,fds_input);
                break;
            case 'o':
                printf("Output function as %s\n",optarg);
                argv_to_socket(optarg,fds_output);
                break;
            case 'b':
                printf("Getting I/O %s\n",optarg);
                argv_to_socket(optarg,fds_input);
                fds_output[0] = fds_input[0];
                fds_output[1] = fds_input[1];
                break;
            case 't':
                muteAlarm = 0;
                timeout = atoi(optarg);
                break;
        }
    }
    int current_pid;
    if (e_is_declared) {
        int pid_ttt = fork();
        if (!pid_ttt) {
            if (!muteAlarm) {
                alarm(timeout);
            }
            int res0 = dup2(fds_input[0], 0);
            if (res0 == -1) {
                perror("dup2");
                return 1;
            }
            int res1 = dup2(fds_output[0], 1);
            if (res1 == -1) {
                perror("dup2");
                return 1;
            }
            execlp(process_path, process_name, process_argv, NULL);
            perror("ttt");
            exit(1);
        }
        else{
            current_pid = pid_ttt;
            printf("ttt pid %d\n",current_pid);
        }
    }
    else{
        int pid_IO = fork();
        if (!pid_IO) {
            if(fds_output[0] == 1){fds_output[0] = 0;}
            if(fds_input[0] == 0){fds_input[0] = 1;}
            char buffer[BUFFER_SIZE] = {'\0'};

            while (1) {
                memset(buffer,0, strlen(buffer));
                int readBytes = read(fds_output[0], buffer, sizeof(char) * BUFFER_SIZE);
                if (readBytes == -1){
                    perror("read");
                    exit(1);
                }
                if (readBytes == 0) {
                    printf("Connection Closed\n");
                    exit(1);
                }

                if(fds_output[0] == fds_input[0]){
                    write(STDOUT_FILENO, buffer, sizeof(char) * strlen(buffer));
                    if(strstr(buffer,"I win") ||strstr(buffer,"I lost") || strstr(buffer,"DRAW")){
                        exit(1);
                    }

                    memset(buffer,0, strlen(buffer));
                    read(STDIN_FILENO, buffer, sizeof(char) *2);
                }
                int writeBytes = write(fds_input[0], buffer, sizeof(char) * strlen(buffer));
                if (writeBytes == -1){
                    perror("write");
                    exit(1);
                }
                if (writeBytes == 0) {
                    printf("Write 0 bytes\n");
                    exit(1);
                }
            }

        }
        else{
            current_pid = pid_IO;
            printf("IO pid %d\n",current_pid);
        }
    }
    wait(NULL);
    printf("Closing Connection... closing pid %d\n",current_pid);
    close(fds_input[0]);
    close(fds_input[1]);
    close(fds_output[0]);
    close(fds_output[1]);
}