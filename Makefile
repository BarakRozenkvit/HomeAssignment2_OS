
all: mync ttt

mync: mync.o
	gcc -Wall mync.o -o mync

ttt: ttt.o
	gcc -Wall ttt.o -o ttt

valgrind: all
	valgrind --leak-check=full ./ttt
	valgrind --leak-check=full ./mync

gcov:
	gcc -Wall -g -fprofile-arcs -ftest-coverage -lgcov -c mync.c
	gcc -Wall -g -fprofile-arcs -ftest-coverage -lgcov -c ttt.c
	gcc -Wall -g -fprofile-arcs -ftest-coverage -lgcov mync.o -o mync
	gcc -Wall -g -fprofile-arcs -ftest-coverage -lgcov ttt.o -o ttt

mync.o: mync.c
	gcc -Wall -c mync.c

ttt.o: ttt.c
	gcc -Wall -c ttt.c

clean:
	rm *.o
	rm mync ttt

cleangcov:
	make clean
	rm *.gcno