#ifndef SM_NFSAPI_H
#define SM_NFSAPI_H

int NFSmount(char *host, char *path);
FILE * NFSopen(char *filename);
int NFSread(void *ptr, int size, FILE * stream);
int NFSclose(FILE * stream);

#endif
