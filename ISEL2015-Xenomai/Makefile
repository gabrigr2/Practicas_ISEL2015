CC=gcc
CFLAGS=-g -Wall -O -I.
LDFLAGS=
LDLIBS=-lrt -lpthread -lwiringPi

all: main

main: main.o tasks.o fsm.o

clean:
	$(RM) *.o *~ main
