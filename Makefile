all: ttt

ttt: tic-tac-toe.o
	gcc -Wall tic-tac-toe.o -o ttt

tic-tac-toe.o: tic-tac-toe.c
	gcc -Wall -c tic-tac-toe.c

clean:
	rm *.o
	rm ttt
