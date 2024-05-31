# Assignment 2 - Operating System
### Goal: Create a netcat program

Running netcat with program:

`./mync -e "<PROGRAM> <ARGS>" ...`

Running netcat on network:
* -b : Get input from socket and output to a connected client
* -i : Get input from socket and output to STDOUT
* -o : Get input from STDIN and output to connected client

Running netcat on TCP Server

`./mync ... -<FLAG> TCPS<PORT>`

Running netcat on TCP Client

`./mync ... -<FLAG> TCPC<HOSTNAME>,<PORT>`

Running netcat on UDP Server

`./mync ... -<FLAG> UDPS<PORT>`

Running netcat on UDP Client

`./mync ... -<FLAG> UDPC<HOSTNAME>,<PORT>`

Running netcat on TCP IOMUX

`./mync ... -<FLAG> TCPMUXS<PORT>`

Running netcat on TCP Unix Domain Server

`./mync ... -<FLAG> UDSSS<PORT>`

Running netcat on TCP Unix Domain Client

`./mync ... -<FLAG> UDSCS<PORT>`




