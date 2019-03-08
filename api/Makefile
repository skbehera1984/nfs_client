CFLAGS = -pedantic

LIBS = -lsocket -lnsl

CC = gcc

all::  nfsapi.o Tester

#  Dependencies. The executable depends on the .o, the .o on the .cpp.

Tester: tester.o nfsapi.o
	$(CC) -o Tester tester.o nfsapi.o $(CFLAGS) $(LIBS)

tester.o: tester.c
	$(CC) -c tester.c $(CFLAGS)

nfsapi: nfsapi.o

nfsapi.o: nfsapi.c
	$(CC) -c nfsapi.c $(CFLAGS)

clean:
	rm *.o Tester
