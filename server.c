#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include  <fcntl.h>
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

int server_setup(int argc, char *argv[]) {

  int sd;
  int i;
  struct sockaddr_in serv_addr, cli_addr, sock;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  error_check( sd, "server socket" );

  portno = atoi(argv[1]);
  sock.sin_family = AF_INET;
  sock.sin_addr.s_addr = INADDR_ANY;
  sock.sin_port = htons(portno);
  i = bind( sd, (struct sockaddr *)&sock, sizeof(sock) );
  error_check( i, "server bind" );

  return sd;
}

int server_connect(int sd) {
  int newsockfd, i;
  struct sockaddr_in sock1, cli_addr;
  char buffer[256]

  i = listen(sd, 5);
  error_check( i, "server listen" );

  unsigned int sock1_len = sizeof(sock1);
  newsockfd = accept( sd, (struct sockaddr *)&sock1, &sock1_len );
  error_check( newsockfd, "server accept" );

  printf("[server] connected to %s\n", inet_ntoa( sock1.sin_addr ) );

  bzero(buffer,256);
  n = read(newsockfd,buffer,255);

  char subbuff[15];
  memcpy( subbuff, &buffer, 14 );
  subbuff[15] = '\0';
  if(strcmp(buffer, "$gitProject -e") == 0){

  }


  return connection;
}

char sendfile(char name){
FILE *fp;
long lSize;
char *buffer;

fp = fopen ( "blah.txt" , "rb" );
if( !fp ) perror("blah.txt"),exit(1);

fseek( fp , 0L , SEEK_END);
lSize = ftell( fp );
rewind( fp );

/* allocate memory for entire content */
buffer = calloc( 1, lSize+1 );
if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

/* copy the file into the buffer */
if( 1!=fread( buffer , lSize, 1 , fp) )
  fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

}

int main() {

  int mainSD = server_setup();

  while(1)
  {
    int branch = server_connect(mainSD);

    int f = fork();
    if(f)
    {
      close(branch);
      continue;
    }

    //this is the main code - use functions from our other files
    //there will be one server executable and one client executable




  }


}
