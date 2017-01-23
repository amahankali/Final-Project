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

void writeFile(char* buffer, char* file)
{
  int fd = open(file, O_TRUNC | O_WRONLY | O_CREAT); //double check
  write(fd, buffer, strlen(buffer));
  close(fd);
}

char getfile(char name){
  FILE *fp;
  long lSize;
  char *buffer;
  fp = fopen ( name , "rb" );
  fseek( fp , 0L, SEEK_END);
  lSize = ftell( fp );
  rewind( fp );
  buffer = calloc( 1, lSize+1 );
  fread( buffer , lSize, 1 , fp) )
  fclose(fp);
  return buffer;
}

char savefile(char name){
  FILE *fp;
  long lSize;
  char *buffer;
  fp = fopen ( name , "rb" );
  fseek( fp , 0L, SEEK_END);
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
      int newsockfd = server_connect(mainSD);

      int f = fork();
      if(f)
      {
        close(branch);
        continue;
      }

      //////////////////Logging in - registers user if needed//////////////////
      char* type = (char *) calloc(1, 1);
      read(newsockfd, type, 1);

      char* username = (char *) calloc(1, MAXUSERNAME);
      char* password = (char *) calloc(1, MAXUSERNAME);

      read(newsockfd, username, MAXUSERNAME); //double check if it blocks with sockets
      read(newsockfd, password, MAXUSERNAME);

      int c;
      if(type == 'r') c = signUp(username, password);
      else c = login(username, password);
      while(!c)
      {
        write(newsockfd, BAD, 1);
        read(newsockfd, username, MAXUSERNAME); //double check if it blocks with sockets
        read(newsockfd, password, MAXUSERNAME);
      }
      write(newsockfd, GOOD, 1);
      /////////////////////////////////////////////////////////////////////////



      /*
      char buffer[256];
      char* username = NULL;
      int option = -1;
      bzero(buffer,256);


      while(1)
      {
          char* n = read(newsockfd,buffer,255);
          if(strcmp(getCommand(n), "$gitProject -L") == 0)
          {
            //Write code for logout
            exit(0);
          }
          if (strcmp(getCommand(n), "$gitProject -e") == 0){//send requested file to client
            char* file;
            char fileName[64];
            filecopy ( fileName, buffer[15], sizeof(buffer) );
            file = getfile(file);
            write(newsockfd, file, strlen(file));
          }
          if (strcmp(getCommand(n), "$gitProject -r") == 0){//receives file from client and saves it on sever
            char* fileText;
            char fileName[64];
            filecopy ( fileName, buffer[15], sizeof(buffer) );
            char file[MAXFILESIZE];
            read(newsockfd,fileText,sizeof(fileText));
            writeFile(fileText, fileName);
          }
      }
      */



  }

  return 0;

}
