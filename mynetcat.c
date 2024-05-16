#include <libc.h>


int main(int argc,char* argv[]){
    int c;
    char* process;
    while ((c = getopt (argc, argv, "e:")) != -1) {
        switch (c) {
            case 'e':
                process = optarg;
                break;
        }
    }
}