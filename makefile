
CFLAGS = -pedantic

# LIBS = -lXt \
#        -lX11 \
#        -lm

LIBS = -lsocket -lnsl

CC = gcc

all:: NFSClient

#  Dependencies. The executable depends on the .o, the .o on the .cpp.


NFSClient: nfsclient.o myrpc/awake.o myrpc/mount_clnt.o myrpc/mount_xdr.o \
	myrpc/nfs_prot_clnt.o myrpc/nfs_prot_xdr.o
	$(CC) -o NFSClient nfsclient.o myrpc/awake.o myrpc/mount_clnt.o \
	myrpc/mount_xdr.o myrpc/nfs_prot_clnt.o myrpc/nfs_prot_xdr.o $(CFLAGS) $(LIBS)

nfsclient.o: nfsclient.c
	$(CC) -c nfsclient.c $(CFLAGS)

clean:
	rm *.o myrpc/*.o
