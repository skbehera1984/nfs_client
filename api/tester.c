#include <stdio.h>
#include <stdlib.h>
#include "nfsapi.h"

int main(int argc, char *argv[])
{
	int boot, filedesc;
	char * host = "eagle";
	char * path = "/playpen1";
	char * file = "matuszek/fiznuckle";
	int amnt = 100;
	FILE * fizile;
	char dataread[1001];

	if (argc > 1)
	{
		host = argv[1];
	}
	if (argc > 2)
	{
		path = argv[2];
	}
	if (argc > 3)
	{
		file = argv[3];
	}
	if (argc > 4)
	{
		amnt = atoi(argv[4]);
	}
	printf("Host is %s\n", host);
	printf("Path is %s\n", path);
	printf("File is %s\n", file);
	printf("Amnt is %d\n", amnt);

	boot = NFSmount(host, path);

	printf("Mount result is %d\n", boot);

	fizile = NFSopen(file);

	printf("Open result is %d\n", fizile->_file);

	boot = NFSread(dataread, amnt, fizile);
	dataread[boot] = '\0';

	printf("Read result is %d\n", boot);
	printf("And it gave data\n%s[end]\n", dataread);

	boot = NFSread(dataread, amnt, fizile);
	dataread[boot] = '\0';

	printf("Read result is %d\n", boot);
	printf("And it gave data\n%s[end]\n", dataread);
	
	boot = NFSclose(fizile);
	printf("Close result is %d\n", boot);

	return 0;
}
