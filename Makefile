
all: mync ttt

mync: mync.o
	gcc -Wall mync.o -o mync

ttt: ttt.o
	gcc -Wall ttt.o -o ttt

mync.o: mync.c
	gcc -Wall -c mync.c

ttt.o: ttt.c
	gcc -Wall -c ttt.c

clean:
	rm *.o
	rm mync ttt