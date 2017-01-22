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

  int sd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  inet_aton(ip, &server.sin_addr);
  int portno = atoi(port);
  server.sin_port = htons(portno);
  
  connect(sd, (struct sockaddr *) &server, sizeof(server));
  return sd;
}

void copyfile(char* file, char* buffer)
{
  int fd = open(file, O_RDONLY);
  read(fd, buffer, MAXFILESIZE);
  close(fd);
}

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
int client_connect( char *host, int argc, char *argv[] ) {
  int sd, i;
  struct sockaddr_in serv_addr hostent *server;
  FILE *fp;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  error_check( sd, "client socket" );

  struct sockaddr_in, sock;
  sock.sin_family = AF_INET;
  inet_aton( host, &(sock.sin_addr));
  portno = atoi(argv[2]);

  server = gethostbyname(argv[1]);
   if (server == NULL) {
       return -1;
   }
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
   serv_addr.sin_port = htons(portno);
   if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
     return -1;



  printf("[client] connecting to: %s\n", host );
  i = connect( sd, (struct sockaddr *)&sock, sizeof(sock) );
  error_check( i, "client connect");

  bzero(buffer,256);
  fp = fopen(argc[3], "r");
  fscanf(fp, "%s", buff);

  n = write(sockfd,buffer,strlen(buffer));
  if (n < 0)
        return -1;
  bzero(buffer,256);
  n = read(sockfd,buffer,255);
  if (n < 0)
    return -1;
  printf("%s\n",buffer);
  return sd;
}
*/
