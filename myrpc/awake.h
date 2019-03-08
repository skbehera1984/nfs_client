#ifndef TRUE
typedef int 	bool;
#define TRUE	1
#define FALSE	0
#endif

#ifndef AWAKE_H
#define AWAKE_H
#include "mount.h"
#include "nfs_prot.h"

int do_NFSmount(char *arguments, int the_socket, diropargs * TMD,
				char * TMDname, char * servername, CLIENT ** clientout);
int do_NFSopen (char *arguments, int the_socket, diropargs * TMD,
				int files_so_far, diropres * stupid, char *server,
				char * TMDname, CLIENT * client);
int do_NFSread (char *arguments, int the_socket, diropres ** diropres_table,
				int * file_offset_table, char * server, CLIENT * client);
int do_NFSclose(char *arguments, int the_socket);
int mountDirectory(char *dirname, diropargs * doaout, char * server);
void prettyprintFILE(FILE * fin, const char * prefix);

#endif
