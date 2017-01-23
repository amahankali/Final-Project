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

//initializes the server
int server_setup(char* port) {

  int sd;
  int i;
  struct sockaddr_in serv_addr, cli_addr, sock;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  error_check( sd, "server socket" );

  int portno = atoi(port);
  sock.sin_family = AF_INET;
  sock.sin_addr.s_addr = INADDR_ANY;
  sock.sin_port = htons(portno);
  i = bind( sd, (struct sockaddr *)&sock, sizeof(sock) );
  error_check( i, "server bind" );

  return sd;
}

//creates a new socket connecting the server to a client
//we use this socket to send and receive file text from the server
//to this particular client
int server_connect(int sd) {
  int newsockfd, i;
  struct sockaddr_in sock1, cli_addr;
  char buffer[256];

  i = listen(sd, 5);
  error_check( i, "server listen" );

  unsigned int sock1_len = sizeof(sock1);
  newsockfd = accept( sd, (struct sockaddr *)&sock1, &sock1_len );
  error_check( newsockfd, "server accept" );

  printf("[server] connected to %s\n", inet_ntoa( sock1.sin_addr ) );
  bzero(buffer,256);
  n = read(newsockfd,buffer,255);

  if (strcmp(getCommand(n), "$gitProject -e") == 0){
    char* file;
    char fileName[64];
    strncpy ( fileName, buffer[15], sizeof(buffer) );
    file = getfile(file);
    write(sd, file, strlen(file));
  }
  return newsockfd;
}

char* getCommand(char* buffer){
  char subbuff[15];
  memcpy( subbuff, &buffer, 14 );
  subbuff[15] = '\0';
  return subbuff;
}

void copyfile(char* file, char* buffer)
{
  int fd = open(file, O_RDONLY);
  read(fd, buffer, MAXFILESIZE);
  close(fd);
}

char getfile(char name){
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



int main() {

  int mainSD = server_setup(PORT);

  while(1)
  {
      int branch = server_connect(mainSD);

      int f = fork();
      if(f)
      {
        close(branch);
        continue;
      }

      char* username = NULL;
      int option = -1;
      /*
        Insert code here that gets the username and password
        and verifies those against the data the server has,
        or registers the user if needed.
      */




  }

  return 0;

}
