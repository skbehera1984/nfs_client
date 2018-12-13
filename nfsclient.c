#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "myrpc/awake.h"
#include "myrpc/mount.h"
#include "myrpc/nfs_prot.h"

#define MAX_REQUEST_SIZE 		1000
#define MAX_SERVERNAME_SIZE		50
#define MAX_DIRNAME_SIZE		150
#define QUEUE_LENGTH 			10
#define NF						256		//  Number of files

#define NFS_MOUNT	1
#define NFS_OPEN	2
#define NFS_READ	3
#define NFS_CLOSE	4

static diropargs * TheMountedDirectory;	//  This represents the mounted directory in
										//    the field "dir". "name" useful?
static int file_offset_table[NF];		//  This is actually all we bother storing.
static diropres * diropres_table[NF];	//  We'll need a diropres{ults} for each one --
										//    that's the "fhandle" and the "fattr"
static bool entry_is_valid[NF];			//  Whether the entries are still valid
static int first_empty_slot = 0;		//  The address of the first empty slot
static char * server;			 		//  The server we mounted from
static char * TMDname;					//  The name of the mounted directory
static CLIENT * client;					//  The NFS RPC client (not the mounting client)

int handle_connection(int the_socket);

int main()
{
	int listening_socket, new_socket;
	int nfs_rpc_socket;					//    int = socket descriptor  

	int returncode, addrlen;			//    very useful 
	int * boot;

	int bindsize, i;

	struct sockaddr listening_address;	//    Local socket 
	struct sockaddr new_address;		//    For child process 
	struct sockaddr_in nfs_rpc_address;	//    Internet socket 

	boot = (int *) malloc (100 * sizeof(int));
	
	server  = (char *) malloc (MAX_SERVERNAME_SIZE * sizeof(char));
	TMDname = (char *) malloc (MAX_DIRNAME_SIZE    * sizeof(char));
	
	for (i=0; i<NF; i++)
	{
		file_offset_table[i] = 0;
		entry_is_valid[i] = FALSE;
	}

  //  The mounted directory, allocate it.

	TheMountedDirectory = (diropargs *) malloc (sizeof(diropargs));
	
  //  The client, allocate it.
  
  	client = (CLIENT *) malloc (sizeof(CLIENT));

  //    Create the local listening socket. 
  //    First, grab a socket descriptor. 	 

	listening_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (listening_socket < 0)
	{
		printf("NFSClient: error %d creating local listening socket\n", errno);
		perror("         ");
		exit(1);
	}
	
  //    Fill the socket info... 

	listening_address.sa_family = AF_UNIX;
	strcpy(listening_address.sa_data, "/tmp/smNFS");

	bindsize = strlen(listening_address.sa_data) + sizeof(listening_address.sa_family);

  //    ...and try and bind it with the name. 
	
	remove("/tmp/smNFS");
	returncode = bind(listening_socket, &listening_address, bindsize);

	if (returncode < 0)
	{
		printf("NFSClient: error %d binding local listening socket\n", errno);
		perror("         ");
		exit(1);
	}
	else
	{
		printf("NFSClient: Opened socket with file descriptor %d\n", listening_socket);
	}
	
  //    Now listen on it... 
		
	returncode = listen(listening_socket, QUEUE_LENGTH);
	if (returncode < 0)
	{
		printf("NFSClient: error %d listening\n", errno);
		perror("         ");
		exit(1);
	}
	else
	{
		printf("NFSClient: Listening on socket %d\n", listening_socket);
	}
	
	//  Yes, unfortunately we are doing this with forking. I'd've  
	//  preferred to figure out select() but I had another project  
	//  due the same day. 
	
	//	It's worse than that -- it doesn't WORK with forking -- I am
	//  instead only going to handle one connection at a time.
	
  //    Ignore child process termination  

	//  signal(SIGCHLD, SIG_IGN);

  //    Go into the infinite loop waiting for connections. 
  
	while (1)
	{
		new_socket = accept(listening_socket, 
			(struct sockaddr *) &new_address, boot);

		if (new_socket < 0)
		{
			printf("NFSClient: error %d accepting\n", errno);
			perror("         ");
			exit(1);
		}
		else
		{
			printf("NFSClient: Connection accepted...\n");
		}
	
		//    Here we fork when accept()ing a connect()ion. 
		//    Have a pretty good handle on this at this point. 
		//    --> IN FACT we don't fork. We'll correct this later. 
	
		if (1 || fork() == 0)			//    Then we're the child... 
		{
			// close(listening_socket);	//    Not needed by child 

			printf("NFSClient: Child started\n");
	
			returncode = handle_connection(new_socket);
			if (returncode < 0)
			{
				printf("NFSClient: error handling connection (as a server)\n", errno);
				exit(1);
			}
			printf("NFSClient: Done handling connection.\n\n");
			
			// exit(0);
		}
		else
		{
			//   printf("NFSClient: Forking...\n"); 
		}
	
	}
  //    Clean up.  

	close(listening_socket);
	remove("/tmp/smNFS");
	printf("NFSClient: Bye bye! :-)\n");
}


//   ##################################################################### 
//   ##################################################################### 


int handle_connection(int the_socket)
{
	int returncode;
	char *buffer;
	char ** lastposition = &buffer;
	char * command;
	char * temp;
	int commandcode;
	int i;
	
	
	buffer = (char *) malloc((MAX_REQUEST_SIZE + 2) * sizeof(char));
	
	printf("NFSClient (handle_connection): Waiting for connection...\n");
		
	returncode = recv(the_socket, buffer, MAX_REQUEST_SIZE, 0);
	if (returncode < 0)
	{
		printf("NFSClient (handle_connection): error %d receiving local message\n", errno);
		perror("      ");
		return(-1);
	}
	else if (returncode == 0)
	{
		printf("NFSClient (handle_connection): Local socket closed by other side...\n");
		return(0);
	}

	// printf("NFSClient (handle_connection): Message received from socket was:\n");

	// buffer[returncode] = '\0';
	// printf("%s[end]\n", buffer);
	
  //	We have to add a newline at the end if there isn't one, dumb eh?
  
	i = strlen(buffer);
	if (buffer[i-1] != '\n')
	{
		// printf("NFSClient (handle_connection): adding a newline to end of args (psht).\n");
		buffer[i] = '\n';
		buffer[i+1] = '\0';
		// printf("NFSClient (handle_connection): args now %s[end]\n", buffer);
	}
	
  //    Now parse the first argument to see what the heck they want. 
	
  //    Get the first token  
	
	temp = (char *) strtok_r(buffer, "\n", lastposition);
	// printf("NFSClient (handle_connection): first arg is %s\n", temp);
	
  //    Parse it as an int representing which function call to make 

	commandcode = atoi(temp);
	
  //    Get the rest of the message into temp	 
	
	temp = (char *) strtok_r(buffer, "", lastposition);
	
	if (temp == NULL)
	{
		printf("NFSClient (handle_connection): That's all that's in the message.\n");
	}
	else
	{
		// printf("NFSClient (handle_connection): And the rest is %s[end]\n", temp);
	}
	
  //    Now pass the rest of the message and the socket to the  
  //    functions that do the work. They will craft the responses 
  //    to the application and send them over the socket. Control 
  //    then comes back here to close the socket. 

	switch(commandcode)
	{
		//  Okay, let's talk about what we're doing in do_NFSmount.
		//  We pass it the rest of the arguments string, for starters.
		//  We also pass it the socket, since it is its job to report
		//  back to the application using the API. We also pass it
		//  TheMountedDirectory, which is a pointer to a "diropargs";
		//  it will fill this with an fhandle which we will need to
		//  call Open and so forth. We also pass it the address of the
		//  server string; it will fill this with the name of the server.
	
		case NFS_MOUNT:	printf("NFSClient (handle_connection): NFSmount called\n");
		
						returncode = do_NFSmount(temp, the_socket, 
							TheMountedDirectory, TMDname, server, &client);
						
						if (returncode == 0)
						{
							printf("NFSClient (handle_connection): Server name is %s\n", server);
							printf("NFSClient (handle_connection): Directory name is %s\n", TMDname);
								
							if (TheMountedDirectory -> name != NULL)
							{
								printf("NFSClient (handle_connection): NFSClient result diropargs.name is: ");
								printf("%s\n", (char *)(TheMountedDirectory->name));
							}
							else
							{
								// printf("NFSClient (handle_connection): NFSClient result has no diropargs.name.\n");
							}
						}
						else
						{
							printf("NFSClient (handle_connection): Dog! Couldn't mount that.\n");
						}
						
						break;
						
		//  Okay, mounting works fine. Now say we want to open a file. 
		//  The luser will give us a filename, which we will assume is
		//  in the directory that luser wanted to mount. (It may include
		//  subdirectory paths.)  We have to return a FILE *. Fortunately
		//  luser cannot luse it as a regular FILE, he can only luse it in
		//  calls to my very own NFSread and NFSclose. So, I can use the 
		//  internal structure of FILE as anything I want. 
		//
		//  We'll get the API part to fake up the FILE * on the fly. All
		//  it needs for that is the file descriptor. We don't even bother
		//  to pass that in, since it's just an index that is determined
		//  out here. 
		//
		//  So what are we going to do when we call NFSopen?  Well, 
		//  obviously we have to call the actual NFS functions. First
		//  we do have to do a LOOKUP, which takes (as always, a pointer
		//  to) a "diropargs" containing the directory filehandle and the
		//  name of the file. This will return a "diropres" * containing
		//  the "fhandle" for the file and an "fattr" which is its 
		//  file attributes. Odds are good that we don't give a hoot about
		//  the file attributes, except maybe its size.
		//
		//  So what do we have to pass to our function? 
		//
		//    A pointer to the "diropargs" containing the fhandle of the
		//    mounted directory, and the name of the file. The function
		//    won't change this but it saves stack space.
		//
		//    Better yet, we can pass the arguments, and then the name
		//    of the file doesn't need to be put in the diropargs by us.
		// 
		//    The name of the server to RPC call to. 
		//
		//  Actually, good, LOOKUP is all that open has to do.
		//
		//  Oops, wait a minute. Am I allowed to use shared memory??? 
		//  Yeah, of course I am, the NFSClient is linked with the functions
		//  that do this stuff, so it'll all be contained in one process. 
		//  It may not be very thread-safe though.
						
		case NFS_OPEN:	printf("NFSClient (handle_connection): NFSopen called\n");

						//  Allocate a diropres to store the result in.
							
						diropres_table[first_empty_slot] = (diropres *)
							malloc(sizeof(diropres));	
							
						//  And make the function call. We pass the args,
						//  the socket, the mounted directory, a pointer
						//  to a FILE * and a diropres * for our results,
						//  and the server name, which we need to make 
						//  the RPC call. 
		
						printf("NFSClient (handle_connection): server name is %s\n", server);
						printf("NFSClient (handle_connection): TMD name is %s\n", TMDname);
							
						returncode = do_NFSopen (temp, the_socket, 
							TheMountedDirectory, first_empty_slot,
							diropres_table[first_empty_slot], 
							server, TMDname, client);

						printf("NFSClient (handle_connection): Ok, NFSopen return code is %d.\n", 
								returncode);
							
						if (returncode == 0)
						{
							printf("NFSClient (handle_connection): That indicates success.\n");
							printf("NFSClient (handle_connection): uid is %d\n", 
								(diropres_table[first_empty_slot])->diropres_u.diropres.attributes.uid);
	
							entry_is_valid[first_empty_slot] = TRUE;
	
							//  Find next empty slot. We do this by incrementing and checking whether
							//  the slot is empty. This is O(n) in the worst case, but the average case
							//  is pretty much constant time.
							
							for (i=0; i < NF; i++)
							{
								if (!entry_is_valid[i])
								{
									first_empty_slot = i;
									break;
								}
							}
							if (i == NF)	//  The whole table was full
							{
								printf("NFSClient ALERT: Too many files are open. I am starting to\n");
								printf("               : clobber files indiscriminately. ALL open files\n");
								printf("               : are in danger and behavior is now UNRELIABLE.\n");
								
								first_empty_slot = 0;
							}
						}
						else
						{
							printf("NFSClient (handle_connection): I'm afraid that is an error.\n");
						}
										
						break;

		//  Okay, well, it would seem we have open working. After a file is opened, it
		//  is stored in our big tables as a diropres (containing its actual file handle)
		//  and as an int representing its logical position.
		//
		//  Thus, if we want to read from a file, all we need is its file descriptor,
		//  which our API will pass us as an int, which do_NFSread will parse. So
		//  what do we pass in?  The pointer to the table of "diropres"es, and the
		//  pointer to the table of offsets. Oh, and of course the server name. We 
		//  actually shouldn't need the currently mounted directory, as the fhandle
		//  ought to be all we need to access the file.
						
		case NFS_READ:	printf("NFSClient (handle_connection): NFSread called\n");
		
						returncode = do_NFSread (temp, the_socket, 
							diropres_table, file_offset_table, server, client);

						printf("NFSClient (handle_connection): Ok, NFSread return code is %d.\n", 
								returncode);
							
						if (returncode == 0)
						{
							printf("NFSClient (handle_connection): That indicates success.\n");
						}
						else
						{
							printf("NFSClient (handle_connection): I'm afraid that is an error.\n");
						}
						
						break;
						
		//  As for closing the file, that is going to be a very simple NFS call.
		//  Note that this is the only function that returns anything besides 0/non0. 
		//  It returns -1 on error and the number of the file to close otherwise.
						
		case NFS_CLOSE:	printf("NFSClient (handle_connection): NFSclose called\n");
		
						returncode = do_NFSclose(temp, the_socket);

						printf("NFSClient (handle_connection): Ok, NFSclose return code is %d.\n", 
								returncode);
							
						if (returncode == -1)
						{
							printf("NFSClient (handle_connection): I'm afraid that is an error.\n");
						}
						else
						{
							entry_is_valid[returncode] = FALSE;
							file_offset_table[returncode] = 0;
						}
						
						break;
		
		default:		printf("NFSClient (handle_connection): Oops -- %d: '%s' called..?\n", commandcode, temp);
						break;
	}
	
	if (returncode < 0)
	{
		printf("NFSClient (handle_connection): Couldn't fulfill request! D'oh.\n");
	}
	else
	{
		printf("NFSClient (handle_connection): Request fulfilled.\n");
	}
	
	printf("NFSClient (handle_connection): Closing.\n");

	//shutdown(the_socket, 2);
	close(the_socket);

	return(0);
}
