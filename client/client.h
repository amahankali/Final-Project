#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void error_check( int i, char *s );
int client_connect( char *host, int argc, char *argv[] );
void copyfile(char* file, char* buffer);
char sendfile(char* name);
int copy(char* buffer, char* filepath);
