CC=gcc
CFLAGS=-g -Wall -O -I. -DNDEBUG
LDFLAGS=
LDLIBS=-lrt -lWiringPi

all: main

main: main.o reactor.o fsm.o

clean:
	$(RM) *.o *~ main
