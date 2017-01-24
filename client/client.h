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

/*
//this is the header file for our command line
//all file names are in reference to current working directory

//share a file
int share(char* filename, char* owner, char* collab, int permissions); //collab is the collaborator

//share a folder
int shareDir(char* foldername, char* owner, char* collab, int permissions);

//make a file
int touch(char* filename);

//make a folder
int mkdir(char* foldername);

//remove a file
int rm(char* filename);

//remove a folder
int rmdir(char* foldername);

//open a file
int edit(char* filename);
*/
