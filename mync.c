#include <unistd.h>
#include <sys/socket.h>
#include <libc.h>


void tcp_server_socket(int port, int* fdsArr){
    int listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 0 means default protocol for stream sockets (Equivalently, IPPROTO_TCP)
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
    serverAddress.sin_addr.s_addr = INADDR_ANY; // any IP at this port (Address to accept any incoming messages)
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

    printf("Waiting for incoming TCP-connections...\n");
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
    fdsArr[2] = clientSocket;
    fdsArr[3] = listeningSocket;
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
    int rval = inet_pton(AF_INET, (const char *)address, &serverAddress.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        exit(1);
    }

    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1) {
        printf("connect() failed with error code");
        // cleanup the socket;
        close(clientSocket);
        exit(1);
    }

    printf("connected to server\n");

    fdsArr[0] = clientSocket;
}

int argv_to_socket(char* str, int* fdsArr){
    int size = strlen(str);
    if(size < 7 || size > 20){
        perror("bad input");
        exit(1);
    }
    char protocol[4] = {str[0],str[1],str[2],'\0'};
    char type = str[3];

    if (strcasecmp(protocol,"TCP")==0){
        if(type == 'S') {
            char* port;
            strcpy(port,str+4);
            tcp_server_socket(atoi(port), fdsArr);
            return 0;
        }
        else if(type == 'C'){
            char* address;
            char* port;
            for(int )
            strcpy(port,str+strlen(strcp));
            //tcp_client_socket(atoi(port),,fdsArr);
            return 0;
        }
        else{
            perror("no type");
            return 0;
        }

    }
    else if(strcasecmp(protocol,"UDP")==0){
        if(type == 'S'){
            //return udp_server_socket(atoi(port));
        }
        else if(type == 'C'){
            return 0;
            //return udp_client_socket(serverAddress,atoi(port));
        }
        else{
            perror("no type");
            return 0;
        }
    }
    else{
        return 0;
    }
    return 0;
}

int main(int argc,char* argv[]){
    char process_path[256];
    getcwd(process_path,sizeof(process_path));
    char process_name[256] = {'/','\0'};
    int fds_input[2] = {0,0};
    int fds_output[2] = {1,1};
    int c=1;
    while (c != -1) {
        c = getopt (argc, argv, "b:o:i:e:");
        switch (c) {
            case 'e':
                strcat(process_name,optarg);
                strncat(process_path,process_name,sizeof(process_path) - strlen(process_path) - 1);
                break;
            case 'i':
                argv_to_socket(optarg,fds_input);
                break;
            case 'o':
                argv_to_socket(optarg,fds_output);
                break;
            case 'b':
                argv_to_socket(optarg,fds_input);
                fds_output[0] = fds_input[0];
                fds_output[1] = fds_input[1];
                break;
        }
    }

    int pid_ttt = fork();
    if(!pid_ttt){
        int res0 = dup2(fds_input[0],0);
        if(res0 == -1){
            perror("dup2");
            return 1;
        }
        int res1 = dup2(fds_output[0],1);
        if(res1 == -1){
            perror("dup2");
            return 1;
        }

        execlp(process_path,process_name,"123456789",NULL);
        perror("ttt");
        exit(1);
    }
    close(fds_input[0]);
    close(fds_input[1]);
    close(fds_output[0]);
    close(fds_output[1]);
    wait(NULL);
}