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
#include <sys/wait.h>
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

int touch(char* filename)
{
  int fd = open(file_name, O_CREAT | O_EXCL);
  if(fd == -1) return 0; //file already exists
  close(fd);
  return 1;
}

int main () {
  int loged = 0;
  char initialBuffer[256]
  int sd = client_connect(TESTIP, TESTPORT);
  printf("Running gitProject\n Enter your username and password to login \n If you wish to sign up please hit enter");
  fgets(initialBuffer, sizeof(initialBuffer), stdin);
  if (initialBuffer[0] == '\0'){
    char newUserName[64];
    char newPassWord[64];
    printf("Input Username\n");
    fgets(newUserName, sizeof(newUserName), stdin);
    printf("Input Password\n", );
    fgets(newPassWord, sizeof(newPassWord), stdin);
    write(sd, "r", 1);
    write(sd, newUserName, sizeof(newUserName));
    write(sd, newPassWord, sizeof(newPassWord));
    printf("You are now signed up" );
    break
  }
  else{
    write(sd, "l", 1);
    char userName[64];
    char passWord[64];
    userName = strtok(initialBuffer," ");
    passWord = strtok(initialBuffer," ");
    write(sd, userName, sizeof(userName));
    write(sd, passWord, sizeof(passWord));

  }

  while(1){
    char buffer[256];
    char subbuff[17];
    scanf("%s", buffer);
    memcpy( subbuff, &buffer, 16 );
    subbuff[17] = '\0';
    char fileName[64];
    if(strcmp(buffer, "$gitProject -lgo") == 0){
      write(sd, buffer, sizeof(buffer));
      exit(0);
    }
    else if(strcmp(subbuff, "$gitProject -crf") == 0){
      write(sd, buffer, sizeof(buffer));

    }
    else if(strcmp(subbuff, "$gitProject -edt") == 0){
      pid_t cpid;
      char text[MAXFILESIZE];
      char returnText[MAXFILESIZE];
      char status[1];
      int status;
      write(sd, buffer, sizeof(buffer));
      read(sd, status, 1);
      strncpy ( fileName, buffer[17], sizeof(buffer) );
      if (strcmp(status, BAD) == 0){
        printf("you do not have permission to access %s", fileName);
      }
      else if{
        read(sd, text, MAXFILESIZE)
      }
      touch(fileName);
      FILE *fp = fopen(fileName, "ab");
      if (fp != NULL)
      {
        fputs(text, fp);
        fclose(fp);
      }
      char *cmd = "emacs";
      char *argv[3];
      argv[0] = "emacs";
      argv[1] = fileName;
      argv[2] = NULL;
      cpid = fork();
      if (c == 0){
        execvp(cmd, argv);
        return 0;
      }
      int status
      wait(&status);
      if(WIFEXITED(status)) {
        copyfile(fileName, returnText);
        write(sd, "$gitProject -rec", sizeof("$gitProject -rec"));
        write(sd, returnText, sizeof(returnText));
        remove(fileName);
      }
    }
    else if (strcmp(buffer, "$gitProject -s") == 0){
      strncpy ( fileName, buffer[17], sizeof(buffer) )
      clientSend();
    }
  }







  return 0;

}
