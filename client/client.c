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

//replaces contents of file named file with buffer
void writeFile(char* buffer, char* file)
{
  int fd = open(file, O_TRUNC | O_WRONLY); //double check
  write(fd, buffer, strlen(buffer));
  close(fd);
}

int touch(char* filename)
{
  int fd = open(file_name, O_CREAT | O_EXCL);
  if(fd == -1) return 0; //file already exists
  close(fd);
  return 1;
}

int main () {

  char initialBuffer[256];

  int sd = client_connect(TESTIP, TESTPORT);

  printf("Running gitProject\n If you wish to sign up please hit enter without typing.\n");
  printf("Otherwise, enter your username and password on one line, separated by a space.\n");
  fgets(initialBuffer, sizeof(initialBuffer), stdin);
  char* nLine = strchr(initialBuffer, '\n'); if(nLine) *nLine = '\0'; //remove new line in initialBuffer

  //////////////////Logging in - registers user if needed//////////////////
  if (initialBuffer[0] == '\0')
  {
    char newUserName[MAXMESSAGE]; bzero(newUserName, MAXMESSAGE);
    char newPassWord[MAXMESSAGE]; bzero(newPassWord, MAXMESSAGE);

    //first reading of username and password
    printf("Input Username\n");
    fgets(newUserName, sizeof(newUserName), stdin);
    nLine = strchr(newUserName, '\n'); if(nLine) *nLine = '\0';
    printf("Input Password\n", );
    fgets(newPassWord, sizeof(newPassWord), stdin);
    nLine = strchr(newPassWord, '\n'); if(nLine) *nLine = '\0';
    /////////////

    write(sd, "r", 1);
    write(sd, newUserName, sizeof(newUserName));
    write(sd, newPassWord, sizeof(newPassWord));

    char resp;
    read(sd, &resp, 1);
    while(strcmp(&resp, BAD) == 0)
    {
      printf("There was a problem with your attempted registration.\n");
      printf("Input Username\n");
      fgets(newUserName, sizeof(newUserName), stdin);
      nLine = strchr(newUserName, '\n'); if(nLine) *nLine = '\0';
      printf("Input Password\n", );
      fgets(newPassWord, sizeof(newPassWord), stdin);
      nLine = strchr(newPassWord, '\n'); if(nLine) *nLine = '\0';

      write(sd, newUserName, sizeof(newUserName));
      write(sd, newPassWord, sizeof(newPassWord));
      read(sd, &resp, 1);
    }

    printf("You are now signed up.\n");
    break;
  }
  else
  {
    write(sd, "l", 1);
    char userName[MAXMESSAGE]; bzero(userName, MAXMESSAGE);
    char passWord[MAXMESSAGE]; bzero(passWord, MAXMESSAGE);

    userName = strtok(initialBuffer, " ");
    nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
    passWord = strtok(initialBuffer, " ");
    nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

    write(sd, userName, sizeof(userName));
    write(sd, passWord, sizeof(passWord));

    char resp;
    read(sd, &resp, 1);
    while(strcmp(&resp, BAD) == 0)
    {
      printf("There was a problem with your attempted login.\n");
      printf("Input Username\n");
      fgets(newUserName, sizeof(newUserName), stdin);
      nLine = strchr(newUserName, '\n'); if(nLine) *nLine = '\0';
      printf("Input Password\n", );
      fgets(newPassWord, sizeof(newPassWord), stdin);
      nLine = strchr(newPassWord, '\n'); if(nLine) *nLine = '\0';

      write(sd, newUserName, sizeof(newUserName));
      write(sd, newPassWord, sizeof(newPassWord));
      read(sd, &resp, 1);
    }

    printf("You are now logged in.\n");
    break;
  }
  /////////////////////////////////////////////////////////////////////////

  while(1)
  {
    char request[MAXMESSAGE + 1]; bzero(request, MAXMESSAGE + 1);
    char command[COMMANDSIZE + 1]; bzero(command, COMMANDSIZE + 1);

    scanf("%s", request);
    memcpy(command, &request, COMMANDSIZE);
    command[COMMANDSIZE] = '\0';

    char fileName[MAXMESSAGE];
    if(strcmp(request, "$gitProject -lgo") == 0){
      write(sd, request, sizeof(request));
      printf("Logged Out\n");
      exit(0);
    }
    else if(strcmp(command, "$gitProject -crf") == 0){
      write(sd, request, sizeof(request));
      char resp;
      read(sd, &resp, 1);
      if(strcmp(&resp, GOOD) == 0) printf("File created!\n");
      else printf("File not created.\n");
    }
    else if(strcmp(command, "$gitProject -edt") == 0){
      char text[MAXFILESIZE]; bzero(text, MAXFILESIZE);
      char returnText[MAXFILESIZE]; bzero(returnText, MAXFILESIZE);

      write(sd, request, MAXMESSAGE);
      char resp;
      read(sd, &resp, 1);

      char* fileName = request + COMMANDSIZE + 1;
      if(strcmp(resp, BAD) == 0){
        printf("You cannot access %s. Either someone is editing it, or it is not shared with you. \n", fileName);
        continue;
      }
      read(sd, text, MAXFILESIZE);

      touch(fileName);
      writeFile(text, fileName);

      char *cmd = "emacs";
      char *argv[3];
      argv[0] = "emacs";
      argv[1] = fileName;
      argv[2] = NULL;

      pid_t cpid;
      cpid = fork();
      if (cpid == 0){
        execvp(cmd, argv);
        return 0;
      }

      int status;
      wait(&status);
      if(WIFEXITED(status)) {
        copyfile(fileName, returnText);
        write(sd, "$gitProject -rec", sizeof("$gitProject -rec"));
        write(sd, “ “, 1);
        write(sd, fileName, sizeof(fileName));
        write(sd, returnText, sizeof(returnText));
        remove(fileName);
      }
      else{
        write(sd, "$gitProject -non”, sizeof("$gitProject”);
        remove(fileName);
      }
    }
    else if (strcmp(command, "$gitProject -rmf") == 0){
      write(sd, request, sizeof(request));
      char resp;
      read(sd, &resp, 1);
      if(strcmp(&resp, GOOD) == 0) printf("File removed!\n");
      else printf("File not removed.\n");
    }
    else if(strcmp (command, "$gitProject -inv") == 0){
      write(sd, request, sizeof(request));
      char resp;
      read(sd, &resp, 1);
      if(strcmp(&resp, GOOD) == 0) printf("Collaborator invited!\n");
      else printf("Collaborator not invited.\n");
    }
    else{
      printf("Request does not match any known command.\n");
    }
  }

  return 0;

}
