all: cshell

cshell: cshell.o
	gcc -Wall -g -o cshell cshell.o

cshell.o: cshell.c
	gcc -Wall -g -o cshell.o -c cshell.c

clean:
	rm -f cshell *.o
