CFLAGS = -pedantic -std=c99 -std=gnu99

# LIBS = -lXt \
#        -lX11 \
#        -lm

LIBS = -lrpcsvc -lnsl

C++ = g++
CC = gcc

all:: Awake mount nfs

#  Dependencies. The executable depends on the .o, the .o on the .cpp.

Awake: awake.o
#	$(CC) -o Awake awake.o mount_clnt.o mount_xdr.o $(LIBS)

awake.o: awake.c
	$(CC) -c awake.c $(CFLAGS)

mount: mount_clnt.o mount_xdr.o

mount_clnt.o: mount_clnt.c
	$(CC) -c mount_clnt.c $(CFLAGS)

mount_xdr.o: mount_xdr.c
	$(CC) -c mount_xdr.c $(CFLAGS)

nfs: nfs_prot_clnt.o nfs_prot_xdr.o

nfs_prot_clnt.o:
	$(CC) -c nfs_prot_clnt.c $(CFLAGS)

nfs_prot_xdr.o:
	$(CC) -c nfs_prot_xdr.c $(CFLAGS)

clean:
	rm *.o
