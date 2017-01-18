#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void error_check( int i, char *s ) {
  if ( i < 0 ) {
    printf("%d\n", i);
    printf("[%s] error %d: %s\n", s, errno, strerror(errno) );
    exit(1);
  }
}


int client_connect( char *host, int argc, char *argv[] ) {
  int sd, i;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  error_check( sd, "client socket" );

  struct sockaddr_in, sock;
  sock.sin_family = AF_INET;
  inet_aton( host, &(sock.sin_addr));
  portno = atoi(argv[1]);
  sock.sin_port = htons(portno);

  printf("[client] connecting to: %s\n", host );
  i = connect( sd, (struct sockaddr *)&sock, sizeof(sock) );
  error_check( i, "client connect");

  return sd;
}
