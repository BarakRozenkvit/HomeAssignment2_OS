#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/errno.h>

#define BUFFER_SIZE 16
#define MAX_SOCKETS 10

int e_is_declared=0;

typedef struct subprocess{
    pid_t pid;
    int socket;
}subprocess;

struct pollfd fds_poll[MAX_SOCKETS];
subprocess pid_to_fds[MAX_SOCKETS];
int available = 0;

void handle_sigchld(int sig) {
    // when signal is alerted do this function, convert pid to client socket
    // and set in poll as assignble with new socket
    pid_t child_pid;
    int status;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for(int i=0;i<=MAX_SOCKETS;i++) {
            if (pid_to_fds[i].pid == child_pid) {
                fds_poll[i].fd = -1;
            }

        }
    }
    if (child_pid == -1 && errno != ECHILD) {
        perror("waitpid");
    }
}

int get_next_slot(){
    // if available got ovrt MAX_SOCKETS search in array for -1
    // and return the index
    available++;
    if(available < MAX_SOCKETS){return available;}
    for(int i=0; i<MAX_SOCKETS;i++){
        if(fds_poll[i].fd == -1){return i;}
    }
    return -1;
}

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

    int listenResult = listen(listeningSocket, MAX_SOCKETS);
    if (listenResult == -1) {
        printf("listen() failed with error code");
        close(listeningSocket);
        exit(1);
    }

    memset(fds_poll,0,sizeof(fds_poll));
    // Assign listening socket to slot 0 in poll
    fds_poll[0].fd = listeningSocket;
    fds_poll[0].events = POLLIN;
    // activate signal SIGCHLD that goes to function when child is dead
    signal(SIGCHLD,handle_sigchld);

    int running=1;

    printf("Server is Waiting for incoming TCP-connections on port: %d...\n",port);
    while(running){
        // Start poll with not timeout == -1
        int ret = poll(fds_poll, MAX_SOCKETS, -1);
        if(ret == -1){
            perror("poll");
            break;
        }

        for(int i=0; i < MAX_SOCKETS && running; i++) {
            // Get New Connections
            if (fds_poll[i].fd == listeningSocket && (fds_poll[i].revents & POLLIN)) {
                struct sockaddr_in clientAddress;  //
                socklen_t clientAddressLen = sizeof(clientAddress);
                memset(&clientAddress, 0, sizeof(clientAddress));
                clientAddressLen = sizeof(clientAddress);
                int clientSocket = accept(listeningSocket, (struct sockaddr *) &clientAddress, &clientAddressLen);
                if (clientSocket == -1) {
                    printf("listen failed with error code");
                    close(listeningSocket);
                    exit(1);
                }
                // Get Next available slot
                int slot = get_next_slot();
                if(slot == -1){
                    printf("Max connections\n");
                    close(clientSocket);
                }
                else {
                    // Put client in poll in slot
                    fds_poll[slot].fd = clientSocket;
                    fds_poll[slot].events = POLLIN;
                    printf("Client %d is Connected\n",slot);
                    // Create child process that breaks this loop and continues to main function
                    // and closes the client socket and listen socket when finishing
                    int pid_new_client = fork();
                    if (!pid_new_client) {
                        fdsArr[0] = clientSocket;
                        fdsArr[1] = listeningSocket;
                        running = 0;
                        break;
                    }
                    // Father process that adds the socket and pid to array and closes the client socket
                    else {
                        subprocess proc;
                        proc.pid = pid_new_client;
                        proc.socket = clientSocket;
                        pid_to_fds[slot] = proc;
                        close(clientSocket);
                    }
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
    // UDP/ TCP / TCPMUSX / USSD
    char protocol[4] = {str[0],str[1],str[2],'\0'};
    // C / S / M
    char type = str[3];

    if(type == 'S'){
        // fill port info
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
    // String for saving current path of executable
    char process_path[256];
    getcwd(process_path,sizeof(process_path));
    // save the process name - ttt and arguments
    char process_name[256] = {'/','\0'};
    char process_argv[256];
    // for strok - split string
    char* token;
    // file descriptors for sockets, deafult is STDIN or STDOUT
    // index 0 - for client socket, index 1 - for listening socket
    int fds_input[2] = {0,0};
    int fds_output[2] = {1,1};
    // argument to alarm for UDP
    int muteAlarm = 1;
    // for running with or without -e
//    int e_is_declared=0;
    // timeout parameter
    int timeout=0;

    int c=1;
    while (c != -1) {
        c = getopt (argc, argv, "t:b:o:i:e:");
        switch (c) {
            case 'e':
                // this fills info for running the program
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
                if(!e_is_declared) {
                    memset(fds_poll,0,sizeof(fds_poll));
                    fds_poll[0].fd = fds_input[0];
                    fds_poll[0].events = POLLIN | POLLHUP;
                }
                break;
            case 'o':
                printf("Output function as %s\n",optarg);
                argv_to_socket(optarg,fds_output);
                if(!e_is_declared) {
                    memset(fds_poll,0,sizeof(fds_poll));
                    fds_poll[0].fd = fds_output[0];
                    fds_poll[0].events = POLLIN | POLLHUP;
                    fds_poll[1].fd = STDIN_FILENO;
                    fds_poll[1].events = POLLIN;
                }
                break;
            case 'b':
                printf("Getting I/O %s\n",optarg);
                argv_to_socket(optarg,fds_input);
                fds_output[0] = fds_input[0];
                fds_output[1] = fds_input[1];
                if(!e_is_declared) {
                    memset(fds_poll,0,sizeof(fds_poll));
                    fds_poll[0].fd = fds_input[0];
                    fds_poll[0].events = POLLIN | POLLHUP;
                    fds_poll[1].fd = STDIN_FILENO;
                    fds_poll[1].events = POLLIN;
                }
                break;
            case 't':
                muteAlarm = 0;
                timeout = atoi(optarg);
                break;
        }
    }
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
    }
    else{
        if (!muteAlarm) {
            alarm(timeout);
        }
        char buffer[BUFFER_SIZE] = {'\0'};
        while (1) {
            int ret = poll(fds_poll, MAX_SOCKETS, -1);
            if(ret == -1){
                perror("poll");
            }

            // if Server closed the connection from socket, break
            if(fds_poll[0].revents & POLLHUP){
                break;
            }
            // if Changed in socket read from socket and write to screen
            else if (fds_poll[0].revents & POLLIN) {
                memset(buffer,0, strlen(buffer));
                int readBytes = read(fds_input[0], buffer, sizeof(char) * BUFFER_SIZE);
                if(readBytes == -1){
                    perror("read");
                    exit(1);
                }
                if (readBytes == 0){
                    printf("Closing Connection...\n");
                    break;
                }

                int writeBytes = write(STDOUT_FILENO, buffer, sizeof(char) * strlen(buffer));
                if(writeBytes <= 0) {
                    perror("write");
                }
            }
            else if(fds_poll[1].revents & POLLIN){
                memset(buffer,0, strlen(buffer));
                int readBytes = read(STDIN_FILENO, buffer, sizeof(char) * BUFFER_SIZE);
                if(readBytes == -1){
                    perror("read");
                    exit(1);
                }
                if (readBytes == 0){
                    printf("Closing Connection...\n");
                    break;
                }

                int writeBytes = write(fds_output[0], buffer, sizeof(char) * strlen(buffer));
                if(writeBytes <= 0){
                    printf("Closing Connection...\n");
                    perror("write");
                }
            }
        }
    }

    wait(NULL);
    printf("Closing Connection...\n");// closing pid %d\n",current_pid);
    close(fds_input[0]);
    close(fds_input[1]);
    close(fds_output[0]);
    close(fds_output[1]);
}