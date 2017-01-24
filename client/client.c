#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "constant.h"

void error_check( int i, char *s ) {
  if ( i < 0 ) {
    printf("%d\n", i);
    printf("[%s] error %d: %s\n", s, errno, strerror(errno) );
    exit(1);
  }
}

int client_connect(char* ip, char* port) {

  int sd = socket(AF_INET, SOCK_STREAM, 0); error_check(sd, "forming socket");
  printf("formed socket\n");

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  inet_aton(ip, &server.sin_addr);
  int portno = atoi(port);
  server.sin_port = htons(portno);

  int ret = connect(sd, (struct sockaddr *) &server, sizeof(server)); error_check(ret, "connecting to server");
  return sd;
}


void copyfile(char* file, char* buffer)
{
  int fd = open(file, O_RDONLY);
  read(fd, buffer, MAXFILESIZE);
  close(fd);
}
/*
char sendfile(char* name){
  FILE *fp;
  long lSize;
  char *buffer;
  fp = fopen ( name , "rb" );
  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );
  buffer = calloc( 1, lSize+1 );
  fread( buffer , lSize, 1 , fp) )
  fclose(fp);
  return buffer;
}

int copy(char* buffer, char* filepath){
  FILE *fp = fopen(filepath, "ab");
  if (fp != NULL){
    fputs(buffer, fp);
    fclose(fp);
  }
}

int clientEdit(int sockfd, char* fileName){

}
*/

int main () {

  int sd = client_connect(TESTIP, TESTPORT);

  char* testBuf = (char *) calloc(1, MAXFILESIZE);

  copyfile("test.txt", testBuf);
  write(sd, testBuf, MAXFILESIZE);
  printf("[CLIENT] test.txt sent\n");

  printf("[CLIENT] about to read\n");

  bzero(testBuf, strlen(testBuf));
  read(sd, testBuf, MAXFILESIZE);
  printf("[CLIENT] read:\n");
  printf("%s\n", testBuf);
  return 0;

}

/*
int main () {

  int sd = client_connect(TESTIP, TESTPORT);

  /*
    Insert code here that gets the username and password
    and sends it to the server for the server to verify.






  return 0;

}
*/
