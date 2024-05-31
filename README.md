# Assignment 2 - Operating System
### Goal: Create a netcat program

Running netcat with program:

`-e "<PROGRAM> <ARGS>"`

Running netcat on network:
* -b : Get input from socket and output to a connected client
* -i : Get input from socket and output to STDOUT
* -o : Get input from STDIN and output to connected client

Running netcat on TCP Server/client

`./mync -b TCP<S/C><PORT>`

Running netcat on UDP Server/client

`./mync -b UDP<S/C><PORT>`

Running netcat on TCP IOMUX

`./mync -b TCPMUXS<PORT>`

Running netcat on TCP Unix Domain Server

`./mync -b UDSSS<PORT>`

Running netcat on TCP Unix Domain Client

`./mync -b UDSCS<PORT>`




