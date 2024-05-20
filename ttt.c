#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int isFinished(int board[3][3]) {
    int countFull = 0;
    for (int i = 0; i < 3; i++) {
        int resHorizontal = 0;
        int resVertical = 0;
        for (int j = 0; j < 3; j++) {
            if(board[i][j] !=0){countFull++;}
            resHorizontal += board[i][j];
            resVertical += board[j][i];
        }

        if (resHorizontal == 3 || resVertical == 3){
            int writeBytes = write(STDOUT_FILENO,"I lost\n",sizeof(char)*7);
            if(writeBytes <= 0) {
                perror("write");
                exit(1);
            }
            //write(STDOUT_FILENO,NULL,0);
            return 1;
        }
        if(resHorizontal == -3 || resVertical == -3) {
            int writeBytes = write(STDOUT_FILENO,"I win\n",sizeof(char)*6);
            if(writeBytes <= 0) {
                perror("write");
                exit(1);
            }
            // Exit Message send nothing
//            write(STDOUT_FILENO,NULL,0);
            return 1;
        }
    }

    int resDiagoanl1 = 0;
    int resDiagonal2 = 0;
    for (int i = 0; i < 3; i++) {
        resDiagoanl1 += board[i][i];
        resDiagonal2 += board[2 - i][i];
    }
    if (resDiagoanl1 == 3 || resDiagonal2 == 3){
        int writeBytes = write(STDOUT_FILENO,"I lost\n",sizeof(char)*7);
        if(writeBytes <= 0) {
            perror("write");
            exit(1);
        }
        // Exit Message send nothing
//        write(STDOUT_FILENO,NULL,0);
        return 1;
    }
    if(resDiagoanl1 == -3 || resDiagonal2 == -3) {
        int writeBytes = write(STDOUT_FILENO,"I win\n",sizeof(char)*6);
        if(writeBytes <= 0) {
            perror("write");
            exit(1);
        }
        // Exit Message send nothing
//        write(STDOUT_FILENO,NULL,0);
        return 1;
    }

    if(countFull == 9){
        int writeBytes = write(STDOUT_FILENO,"DRAW\n",sizeof(char)*5);
        if(writeBytes <= 0) {
            perror("write");
            exit(1);
        }
        // Exit Message send nothing
//        write(STDOUT_FILENO,NULL,0);
        return 1;
    }
    return 0;
}

void ticTacToe(char* stratagy,size_t size){
    int board[3][3] = {{0}};
    int p = 0;

    while (1) {
        // Computer Turn
        while (p<size){
            int chooseComp = stratagy[p] - '0';
            int i = (chooseComp-1)/3;
            int j = (chooseComp-1) - 3*i;
            if (!board[i][j]) {
                board[i][j] = -1;
                char out[2] = {stratagy[p],'\n'};
                int writeBytes = write(STDOUT_FILENO,out,sizeof(char)*2);
                if(writeBytes <= 0){
                    perror("write");
                    exit(1);
                }
                break;
            }
            p++;
        }

        if(isFinished(board)){
            break;
        }
        while (1) {
            // User Turn
            int chooseUser;
            char in[2];
            int readBytes = read(STDIN_FILENO,in,sizeof(char)*2);
            if(readBytes<=0){
                write(STDOUT_FILENO,NULL,0);
                return;
            }
            chooseUser = atoi(in);
            if (chooseUser <= 0 || chooseUser > 9) {
                continue;
            }

            int i = (chooseUser-1)/3;
            int j = (chooseUser-1) - 3*i;
            if (!board[i][j]) {
                board[i][j] = 1;
                break;
            }
        }

        if(isFinished(board)){
            break;
        }
    }
}


int main(int argc, char* argv[]) {
    if(argc != 2){
        perror("Error\n");
        return 1;
    }

    size_t size = strlen(argv[1]);
    if(size < 9 || size > 9){
        perror("Error\n");
        return 1;
    }

    int countDigits[9] = {0};
    for(int i=0;i<size;i++){
        int digit = argv[1][i] - '0';
        if(digit <= 0 || digit > 9){
            perror("Error\n");
            return 1;
        }

        if(!countDigits[digit-1]){
            countDigits[digit-1]++;
        }

        else{
            perror("Error\n");
            return 1;
        }
    }
    ticTacToe(argv[1],size);
}
