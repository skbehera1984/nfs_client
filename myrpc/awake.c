#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "mount.h"
#include "nfs_prot.h"
#include "awake.h"

int do_NFSmount(char *arguments, int the_socket, diropargs * TMD,
				char * TMDname, char * servername, CLIENT ** clientout)
{
	char ** lastposition = &arguments;
	char * temp;
	char * temp2;
	char * message;
	int bytes_sent;

	CLIENT * client;
	fhstatus * result;
	dirpath * mountpath;
	char * server;
	int intresult;
	int i;
	
  //  Get the host and pathnames
	
	temp = (char *) strtok_r(arguments, "\n", (char **) lastposition);
	printf("do_NFSmount: temp is %s\n", temp);
	
	server = temp;
	
	temp2 = (char *) strtok_r(arguments, "\n", (char **) lastposition);
	printf("do_NFSmount: temp2 is %s\n", temp2);

	mountpath = &temp2;
	strcpy(TMDname, *mountpath);
	
	printf("do_NFSmount: server is %s\n", server);
	printf("do_NFSmount: *mountpath is %s\n", *mountpath);

  //  Do the mounting
  
	client = clnt_create(server, MOUNTPROG, MOUNTVERS, "udp");
	if (client == NULL)
	{
		printf("do_NFSmount: Couldn't create the MOUNTING client");
		clnt_pcreateerror(server);
		//exit(1);
		intresult = -1;
	}
	else
	{
		client->cl_auth = authunix_create_default();

		result = mountproc_mnt_1(mountpath, client);
		
		intresult = result->fhs_status;
		printf("do_NFSmount: Result status is %d\n", intresult);
	
	  //  Okay, so what have we actually accomplished here?  We have
	  //  made the RPC call that mounts the directory. That gets us
	  //  the first file handle (the handle of the directory we mounted).
	  //  The return value (result) of the call is something called an
	  //  "fhstatus", which contains, although I don't get unions really:
	  //  
	  //    An integer which is the status of the call. Zero means success.
	  //    An "fhandle" which is the directory we've mounted.
	  //
	  //  Apparently we can use the fhandle of the directory to open up
	  //  files now. So we'll copy that fhandle into our "diropargs" TMD,
	  //  which is TheMountedDirectory, which is state kept by the NFS
	  //  client thingie (and wouldn't it be nice to do this in an object-
	  //  oriented language?)  We also need to pass out the name of the 
	  //  server.
	
		if (intresult != 0)
		{
			printf("do_NFSmount: That indicates some kind of error... sorry.\n");
		}
		else
		{
			memcpy(TMD->dir.data, result->fhs_fh, NFS_FHSIZE);
		}
	
	  //  Okay, we are also going to create an NFS client and pass it back
	  //  up, so we don't have to create a new NFS client every time.
		
		*clientout  = clnt_create(server, NFS_PROGRAM, NFS_VERSION, "udp");
		if (*clientout == NULL)
		{
			printf("do_NFSmount: Couldn't create the NFS client");
			clnt_pcreateerror(server);
			exit(1);
		}
		else
		{
			(*clientout)->cl_auth = authunix_create_default();
		}
		
		strcpy(servername, server);
		
	}
	
  //  We do one more thing, which is send a message containg the
  //  return code (0 or -1) to the application that asked us to
  //  make the call.
  
	message = (char *) malloc (100 * sizeof(char));

	if (intresult == 0)
	{
		strcat(message, "0");
	}
	else
	{
		strcat(message, "-1");
	}	

	bytes_sent = send(the_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		printf("do_NFSmount: error %d sending message\n", errno);
		perror("           ");  
		exit(1);
	}
	
	printf("do_NFSmount: Done.\n");

	return(intresult);
}


int do_NFSopen (char *arguments, int the_socket, diropargs * TMD,
				int files_so_far, diropres * dorout, char * server,
				char * TMDname, CLIENT * client)
{
	char ** lastposition = &arguments;
	char * temp;
	char * message;
	char * dirname;
	char * lastslash;
	int bytes_sent;
	diropres * dirresult;
	int intresult = -1;
	int slashfound = FALSE;
	diropargs * subdir;
	int mdreturncode;
	
  //  Get the filename

	printf("do_NFSopen: server name is %s\n", server);
	printf("do_NFSopen: TMDname is %s\n", TMDname);

	temp = (char *) strtok_r(arguments, "\n", (char **) lastposition);

  //  First, check for /s in the filename. If there are any, we need 
  //  to mount another directory.
  
	if (temp != NULL)
	{
		lastslash = strrchr(temp, '/');
		
		if (lastslash != NULL)
		{
			slashfound = TRUE;
			lastslash[0] = '\0';
			
			printf("do_NFSopen: directory name is %s, filename is %s\n", 
					temp, lastslash + 1);
			
			dirname = (char *) malloc ((strlen(TMDname)+strlen(temp)+4) * sizeof(char));
			strcat(dirname, TMDname);
			strcat(dirname, "/");
			strcat(dirname, temp);
			
			printf("do_NFSopen: dirname is %s\n", dirname);
			
			subdir = (diropargs *) malloc (sizeof(diropargs));
			mdreturncode = mountDirectory(dirname, subdir, server);
			
			printf("do_NFSopen: mountdir return code is %d\n", mdreturncode);
			
			subdir -> name = lastslash + 1;
		}
		else
		{
			printf("do_NFSopen: filename is %s\n", temp);
	
		  //  Put the file name into the diropargs for lookup.
  
  			TMD -> name = temp;
		}
	}
	
	if (temp == NULL)
	{
		printf("do_NFSopen: No arguments found.\n");
	}	
	else if (client == NULL)
	{
		printf("do_NFSopen: Couldn't use the client.\n");
		printf("do_NFSopen: Did you remember to mount first?\n");
		clnt_pcreateerror(server);
	}
	else
	{
	  //  Make the call. Note that for now we are only handling files on 
	  //  the top level. Boy did the Red Sox just get jobbed. AGAIN.
	  //  Anyway, the call returns a pointer to a "diropres", which holds
	  //  a status code and an fhandle, the fhandle being what we actually
	  //  need. We have to put them in "dorout" which is the outgoing diropres.

		if (slashfound)
		{
			dirresult = nfsproc_lookup_2(subdir, client);	
		}
		else
		{	 
			dirresult = nfsproc_lookup_2(TMD, client);	
		}
		
		intresult = dirresult -> status;
		printf("do_NFSopen: Dirresult status is %d\n", intresult);
		
		memcpy(dorout, dirresult, sizeof(diropres));
		
		// printf("do_NFSopen: uid is %d\n", dirresult->diropres_u.diropres.attributes.uid);
		// printf("do_NFSopen: uid is %d\n", dorout->diropres_u.diropres.attributes.uid);
		
		// printf("do_NFSopen: file desc of file is %d\n", (*fileout)->_file);
				
	  //  Ok, just one more thing, then. We have to send the FILE * 
	  //  over the socket to the luser. Except that I have a better
	  //  idea -- we can just send the file descriptor over. All the
	  //  user gets to do with its pretend FILE * is pass it to 
	  //  my own functions, and they can keep track for themselves.	  
	}
	
	message = (char *) malloc (100 * sizeof(char));

	if (client == NULL || intresult != 0)
	{
		strcat(message, "-1");
	}
	else
	{
		sprintf(message, "%d", files_so_far);
	}

	bytes_sent = send(the_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		printf("do_NFSmount: error %d sending message\n", errno);
		perror("           ");  
		exit(1);
	}
	
	return(intresult);
}


int do_NFSread (char *arguments, int the_socket, diropres ** diropres_table, 
				int * file_offset_table, char * server, CLIENT * client)
{	
	char ** lastposition = &arguments;
	char * temp;
	char * message;
	int bytes_sent;

	readres * readresult;
	readargs * thearguments;

	int intresult = -1;
	int filedesc, amount;
	int argsgood = 1;
	int i, amount_received;
	int logicalposition;

	char * filename;
	char numberstring[8] = "\0\0\0\0\0\0\0\0";
	int numberstringlength;
	
  //  What arguments do we expect this function to need?  A file descriptor
  //  (int) saying which file to read from. And an int saying how many bytes
  //  to read. (NOT the offset, which the user doesn't get to do a darn thing
  //  about.)

  //  Get the file descriptor -- should be the first argument.

	printf("do_NFSread: server name is %s\n", server);
	
	temp = (char *) strtok_r(arguments, "\n", (char **) lastposition);
	if (temp == NULL)
	{
		printf("do_NFSread: No arguments found.\n");
		argsgood = 0;
	}
	else
	{
		// printf("do_NFSread: temp is %s\n", temp);
		filedesc = atoi(temp);
		printf("do_NFSread: filedesc is %d\n", filedesc);
	}
	
  //  Get the amount -- should be the second argument.
 	
	temp = (char *) strtok_r(arguments, "\n", (char **) lastposition);
	if (temp == NULL)
	{
		printf("do_NFSread: Not enough arguments.\n");
		argsgood = 0;
	}
	else
	{
		// printf("do_NFSread: temp is %s\n", temp);
		amount = atoi(temp);
		printf("do_NFSread: amount is %d\n", amount);

		if (amount > 8000)
		{
			printf("do_NFSread: but I'm afraid I can only give you 8000 bytes.\n");
			amount = 8000;
		}
		else if (amount < 1)
		{
			printf("do_NFSread: which I guess means you didn't want any data.\n");
			argsgood = 0;
		}
	}
	
	if (argsgood)
	{
	  //  Check the client
	  
		if (client == NULL)
		{
			printf("do_NFSread: Couldn't use the client.\n");
			printf("do_NFSread: Did you remember to mount first?\n");
			clnt_pcreateerror(server);
		}
		else
		{
			printf("do_NFSread: Allocating readargs structures.\n");
		
			thearguments = (readargs *) malloc (sizeof(readargs));
	
			logicalposition = file_offset_table[filedesc];

			thearguments -> offset = logicalposition;
			thearguments -> count = amount;
			thearguments -> totalcount = 0;
			memcpy(thearguments -> file.data, 
				(diropres_table[filedesc]) -> diropres_u.diropres.file.data,
				NFS_FHSIZE);
			
			printf("do_NFSread: Trying to read from file number %d\n", filedesc);

			readresult = nfsproc_read_2(thearguments, client);
	
			intresult = readresult -> status;
			printf("do_NFSread: int result is %d\n", intresult);
		
			//  The number of bytes we got is stored in the secret
			//  location readres->readres_u.reply.data.data_len, 
			//  right next to the data itself. (Annoying!)

			if (intresult == 0)
			{		
				amount_received = readresult -> readres_u.reply.data.data_len;
	
				printf("do_NFSread: got %d bytes (?)\n", amount_received);
	
				//  Update the logical position of the file.
				//  If it's equal to or greater than the size of the
				//  file, reset it to zero.
				//
				//  No, actually, don't. We do that on close.

				logicalposition += amount_received;
				if (logicalposition >= 
				    readresult -> readres_u.reply.attributes.size)
				{
					// logicalposition = 0;
				}
				file_offset_table[filedesc] = logicalposition;

				//  Craft the response to be sent over the socket

				sprintf(numberstring, "%d\n", amount_received);
				numberstringlength = strlen(numberstring);

				printf("do_NFSread: numberstring is %d chars long ",
					numberstringlength);
				printf("do_NFSread: and is \n%s[end]\n",
					numberstring);

				message = (char *) malloc (
					(10 + amount_received) * sizeof (char));
				bzero(message, (10+amount_received));
	
				strcat(message, numberstring);

				memcpy((message+numberstringlength), 
					readresult -> readres_u.reply.data.data_val,
					amount_received);
				
				// printf("--------------\n");
				// printf("%s\n", message);
				// printf("--------------\n");

			}
		}
	}
	
	if (client == NULL || intresult != 0)
	{
		// message = (char *) malloc (8 * sizeof(char));
		// strcat(message, "-1\n");
		message = "-1\n";
	}
	else
	{
		// message = (char *) malloc ((10 + amount_received) * sizeof (char));

		// sprintf(message, "%d", am);
	}

	bytes_sent = send(the_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		printf("do_NFSmount: error %d sending message\n", errno);
		perror("           ");  
		exit(1);
	}
	
	if (intresult != 0)
	{
		return (intresult);
	}
	else if (!argsgood)
	{
		return -1;
	}

	return 0;
}



//  NFSclose only figures out the file number to close and passes it
//  back up.

int do_NFSclose (char *arguments, int the_socket)
{
	char ** lastposition = &arguments;
	char * temp;
	int filedesc;

	int bytes_sent;

	char * message = "0\n";
	
  //  Get the file descriptor -- should be the first argument.

	temp = (char *) strtok_r(arguments, "\n", (char **) lastposition);
	if (temp == NULL)
	{
		printf("do_NFSclose: No arguments found.\n");
		filedesc = -1;
	}
	else
	{
		filedesc = atoi(temp);
		printf("do_NFSclose: filedesc is %d\n", filedesc);
	}

	bytes_sent = send(the_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		printf("do_NFSmount: error %d sending message\n", errno);
		perror("           ");  
		exit(1);
	}
	return (filedesc);
}


void prettyprintFILE(FILE * fin, const char * prefix)
{
	printf("%sFile descriptor is %d, logical position is %d\n",
			prefix, fin->_file, fin->_cnt);
}


int mountDirectory(char *dirname, diropargs * doaout, char * server)
{
	CLIENT * client;
	fhstatus * result;
	dirpath * mountpath;
	int i, intresult;
	
  //  Get the host and pathnames
	
	mountpath = &dirname;
	
	printf("mountDirectory: server is %s\n", server);
	printf("mountDirectory: *mountpath is %s\n", *mountpath);

  //  Do the mounting
  
	client = clnt_create(server, MOUNTPROG, MOUNTVERS, "udp");
	if (client == NULL)
	{
		printf("mountDirectory: Couldn't create the MOUNTING client");
		clnt_pcreateerror(server);
		return -1;
	}
	else
	{
		client->cl_auth = authunix_create_default();

		result = mountproc_mnt_1(mountpath, client);
		
		intresult = result->fhs_status;
		printf("mountDirectory: Result status is %d\n", intresult);
	
		if (intresult != 0)
		{
			printf("mountDirectory: That indicates some kind of error... sorry.\n");
		}
		else
		{
			memcpy(doaout->dir.data, result->fhs_fh, NFS_FHSIZE);
		}
		return 0;
	}
}






