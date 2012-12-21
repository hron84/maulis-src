# simple makefile for talk.c

CC=gcc
LDFLAGS=-lnsl -lncurses
# libc5: -lsocket
LD=gcc

default: talk
talk: talk.o readline.o
	$(LD) $(LDFLAGS) -o talk talk.o readline.o
talk.o: talk.c readline.h
	$(CC) -c -o talk.o talk.c
readline.o: readline.c readline.h
	$(CC) -c -o readline.o readline.c

clean:
	rm -f *.o talk
