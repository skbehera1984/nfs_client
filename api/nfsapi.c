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

#define MAX_MESSAGE_LENGTH	128
#define MAX_RESPONSE_SIZE	10000

#define API_DEBUG 	0


int NFSmount(const char *host, const char *path)
{
	int the_other_socket;
	struct sockaddr the_other_address;

	int returncode, intresult;
	int bytes_sent;
	int connectsize;

	char *buffer;
	
	char message[MAX_MESSAGE_LENGTH];
	bzero(message, MAX_MESSAGE_LENGTH);
	
   //  First, grab a socket descriptor. 	

	the_other_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (the_other_socket < 0)
	{
		printf("NFSmount: error %d creating local socket\n", errno);
		perror("      ");
		exit(1);
	}
	
  //  Fill the socket info...

	the_other_address.sa_family = AF_UNIX;
	strcpy(the_other_address.sa_data, "/tmp/smNFS");

	connectsize = strlen(the_other_address.sa_data) +
		sizeof(the_other_address.sa_family);

  //  ...and try and bind it with the name.
	
	returncode = connect(the_other_socket, &the_other_address, connectsize);

	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSmount: error %d connecting to local socket\n", errno);
		perror("      ");
		exit(1);
	}

  //  Build a message according to the protocol: mount goes
  //  1
  //  host
  //  path
	
	strcat(message, "1\n");
	strcat(message, host);
	strcat(message, "\n");
	strcat(message, path);
	strcat(message, "\n");

	if (API_DEBUG) printf("NFSmount: Message is:\n%s[end]\n", message);

	if (API_DEBUG) printf("NFSmount: Connected, sending message...\n");
	
	bytes_sent = send(the_other_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		if (API_DEBUG) printf("NFSmount: error %d sending message\n", errno);
		perror("      ");  
		exit(1);
	}

  //  Get the return message.
	
	buffer = (char *) malloc((MAX_RESPONSE_SIZE + 1) * sizeof(char));
	
	if (API_DEBUG) printf("NFSmount: Waiting for connection...\n");
		
	returncode = recv(the_other_socket, buffer, MAX_RESPONSE_SIZE, 0);
	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSmount: error %d receiving local message\n", errno);
		perror("      ");
		return(-1);
	}
	else if (returncode == 0)
	{
		if (API_DEBUG) printf("NFSmount: Local socket closed by other side...\n");
		return(-1);
	}

	if (API_DEBUG) printf("NFSmount: Message received (and ignored) from socket was:\n");

	buffer[returncode] = '\0';
	if (API_DEBUG) printf("%s\n", buffer);

  //  Should be just one integer. If it's zero, we're good.
  //  Else we return error.
  
  	intresult = atoi(buffer);
  	
 	if (API_DEBUG) printf("code is %d\n", intresult);
	
	close(the_other_socket);
	
	return(intresult);
}

//----------------------------------------------------------------------


FILE * NFSopen(char *filename)
{
	int the_other_socket;
	struct sockaddr the_other_address;

	int returncode, intresult;
	int bytes_sent;
	int connectsize;

	char *buffer;
	
	FILE * result;
	
	char message[MAX_MESSAGE_LENGTH];
	bzero(message, MAX_MESSAGE_LENGTH);
	
   //  First, grab a socket descriptor. 	

	the_other_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (the_other_socket < 0)
	{
		printf("NFSopen: error %d creating local socket\n", errno);
		perror("      ");
		exit(1);
	}
	
  //  Fill the socket info...

	the_other_address.sa_family = AF_UNIX;
	strcpy(the_other_address.sa_data, "/tmp/smNFS");

	connectsize = strlen(the_other_address.sa_data) +
		sizeof(the_other_address.sa_family);

  //  ...and try and bind it with the name.
	
	returncode = connect(the_other_socket, &the_other_address, connectsize);

	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSopen: error %d connecting to local socket\n", errno);
		perror("      ");
		exit(1);
	}

  //  Build a message according to the protocol: open goes
  //  2
  //  filename
	
	strcat(message, "2\n");
	strcat(message, filename);
	strcat(message, "\n");

	if (API_DEBUG) printf("NFSopen: Message is:\n%s[end]\n", message);

	if (API_DEBUG) printf("NFSopen: Connected, sending message...\n");
	
	bytes_sent = send(the_other_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		if (API_DEBUG) printf("NFSopen: error %d sending message\n", errno);
		perror("      ");  
		exit(1);
	}
	
	buffer = (char *) malloc((MAX_RESPONSE_SIZE + 1) * sizeof(char));
	
	if (API_DEBUG) printf("NFSopen: Waiting for connection...\n");
		
	returncode = recv(the_other_socket, buffer, MAX_RESPONSE_SIZE, 0);
	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSopen: error %d receiving local message\n", errno);
		perror("      ");
		return(NULL);
	}
	else if (returncode == 0)
	{
		if (API_DEBUG) printf("NFSopen: Local socket closed by other side...\n");
		return(NULL);
	}

	if (API_DEBUG) printf("NFSopen: Message received (and ignored) from socket was:\n");

	buffer[returncode] = '\0';
	if (API_DEBUG) printf("%s\n", buffer);
	
  //  Should be just one integer. If it's zero, we're good.
  //  Else we return error.
  
  	intresult = atoi(buffer);
  	
 	if (API_DEBUG) printf("code is %d\n", intresult);

	result = (FILE *) malloc (sizeof(FILE));
	
	result -> _file = intresult;
	result -> _ptr  = NULL;
	result -> _base = NULL;
	result -> _flag = 0;
	result -> _cnt  = 0;
	
	close(the_other_socket);
	
	return(result);
}

//----------------------------------------------------------------------


int NFSread(void *ptr, int size, FILE * stream)
{
	int the_other_socket;
	struct sockaddr the_other_address;

	int returncode, intresult;
	int bytes_sent;
	int connectsize;

	char *buffer;
	char temp[12];
	
	char message[MAX_MESSAGE_LENGTH];
	bzero(message, MAX_MESSAGE_LENGTH);
	
   //  First, grab a socket descriptor. 	

	the_other_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (the_other_socket < 0)
	{
		printf("NFSread: error %d creating local socket\n", errno);
		perror("      ");
		exit(1);
	}
	
  //  Fill the socket info...

	the_other_address.sa_family = AF_UNIX;
	strcpy(the_other_address.sa_data, "/tmp/smNFS");

	connectsize = strlen(the_other_address.sa_data) +
		sizeof(the_other_address.sa_family);

  //  ...and try and bind it with the name.
	
	returncode = connect(the_other_socket, &the_other_address, connectsize);

	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSread: error %d connecting to local socket\n", errno);
		perror("      ");
		exit(1);
	}

  //  Build a message according to the protocol: read goes
  //  3
  //  file descriptor
  //  amount
	
	strcat (message, "3\n");
	sprintf(temp, "%d\n", stream -> _file);
	strcat (message, temp);
	sprintf(temp, "%d\n", size);
	strcat (message, temp);

	if (API_DEBUG) printf("NFSread: Message is:\n%s[end]\n", message);

	if (API_DEBUG) printf("NFSread: Connected, sending message...\n");
	
	bytes_sent = send(the_other_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		if (API_DEBUG) printf("NFSread: error %d sending message\n", errno);
		perror("      ");  
		exit(1);
	}
	
	buffer = (char *) malloc((MAX_RESPONSE_SIZE + 1) * sizeof(char));
	
	if (API_DEBUG) printf("NFSread: Waiting for connection...\n");
		
	returncode = recv(the_other_socket, buffer, MAX_RESPONSE_SIZE, 0);
	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSread: error %d receiving local message\n", errno);
		perror("      ");
		return(-1);
	}
	else if (returncode == 0)
	{
		if (API_DEBUG) printf("NFSread: Local socket closed by other side...\n");
		return(-1);
	}

	if (API_DEBUG) printf("NFSread: Message received (and ignored) from socket was:\n");

	buffer[returncode] = '\0';
	if (API_DEBUG) printf("%s\n", buffer);
	
  //  Should be just one integer. If it's zero, we're good.
  //  Else we return error.
  
  	intresult = atoi(buffer);
  	
 	if (API_DEBUG)
 	{
 		printf("code is %d\n", intresult);
 		printf("the data is %s[end]\n", strchr(buffer, '\n')+1);
 		printf("its size is %d\n", strlen(strchr(buffer, '\n')+1));
 	}
 			 	
 	if (intresult < 0)
 		return(-1);

  //  Fill the void ptr. 
  
  	memcpy(ptr, strchr(buffer, '\n')+1, size);
	
	close(the_other_socket);
	
	return(intresult);
}


//----------------------------------------------------------------------


int NFSclose(FILE * stream)
{
	int the_other_socket;
	struct sockaddr the_other_address;

	int returncode, intresult;
	int bytes_sent;
	int connectsize;

	char *buffer;
	char temp[12];
	
	char message[MAX_MESSAGE_LENGTH];
	bzero(message, MAX_MESSAGE_LENGTH);
	
   //  First, grab a socket descriptor. 	

	the_other_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (the_other_socket < 0)
	{
		printf("NFSclose: error %d creating local socket\n", errno);
		perror("      ");
		exit(1);
	}
	
  //  Fill the socket info...

	the_other_address.sa_family = AF_UNIX;
	strcpy(the_other_address.sa_data, "/tmp/smNFS");

	connectsize = strlen(the_other_address.sa_data) +
		sizeof(the_other_address.sa_family);

  //  ...and try and bind it with the name.
	
	returncode = connect(the_other_socket, &the_other_address, connectsize);

	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSclose: error %d connecting to local socket\n", errno);
		perror("      ");
		exit(1);
	}

  //  Build a message according to the protocol: close goes
  //  4
  //  file descriptor
	
	strcat (message, "4\n");
	sprintf(temp, "%d\n", stream -> _file);
	strcat (message, temp);

	if (API_DEBUG) printf("NFSclose: Message is:\n%s[end]\n", message);

	if (API_DEBUG) printf("NFSclose: Connected, sending message...\n");
	
	bytes_sent = send(the_other_socket, message, strlen(message), 0);
	
	if (bytes_sent < 0)   
	{
		if (API_DEBUG) printf("NFSclose: error %d sending message\n", errno);
		perror("      ");  
		exit(1);
	}
	
	buffer = (char *) malloc((MAX_RESPONSE_SIZE + 1) * sizeof(char));
	
	if (API_DEBUG) printf("NFSclose: Waiting for connection...\n");
		
	returncode = recv(the_other_socket, buffer, MAX_RESPONSE_SIZE, 0);
	if (returncode < 0)
	{
		if (API_DEBUG) printf("NFSclose: error %d receiving local message\n", errno);
		perror("      ");
		return(-1);
	}
	else if (returncode == 0)
	{
		if (API_DEBUG) printf("NFSclose: Local socket closed by other side...\n");
		return(-1);
	}

	if (API_DEBUG) printf("NFSclose: Message received (and ignored) from socket was:\n");

	buffer[returncode] = '\0';
	if (API_DEBUG) printf("%s\n", buffer);

  //  Should be just one integer. If it's zero, we're good.
  //  Else we return error.
  
  	intresult = atoi(buffer);
  	
 	if (API_DEBUG) printf("code is %d\n", intresult);
	
	close(the_other_socket);
	
	return(intresult);
}












