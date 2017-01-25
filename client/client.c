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
    printf("[%s] client error %d: %s\n", s, errno, strerror(errno) );
    exit(1);
  }
}

//error checking for fgets
void error_checkF(int i, char *s ) {
  if(i <= 0) {
    printf("%d\n", i);
    printf("[%s] client error %d: %s\n", s, errno, strerror(errno) );
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
  int fd = open(filename, O_CREAT | O_EXCL);
  if(fd == -1) return 0; //file already exists
  close(fd);
  return 1;
}

int main () {

  char initialBuffer[256];

  int sd = client_connect(TESTIP, TESTPORT);

  printf("Running gitProject\n If you wish to sign up please hit enter without typing.\n");
  printf("Otherwise, enter your username and password on one line, separated by a space.\n");
  int v = fgets(initialBuffer, sizeof(initialBuffer), stdin); error_checkF(v, "Reading first communication");
  char* nLine = strchr(initialBuffer, '\n'); if(nLine) *nLine = '\0'; //remove new line in initialBuffer

  //////////////////Logging in - registers user if needed//////////////////
  if (initialBuffer[0] == '\0')
  {
    char userName[MAXMESSAGE]; bzero(userName, MAXMESSAGE);
    char passWord[MAXMESSAGE]; bzero(passWord, MAXMESSAGE);

    //first reading of username and password
    printf("Input Username\n");
    v = fgets(userName, sizeof(userName), stdin); error_checkF(v, "line 90");
    nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
    printf("Input Password\n");
    v = fgets(passWord, sizeof(passWord), stdin); error_checkF(v, "line 93");
    nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';
    /////////////

    v = write(sd, "r", 1); error_check(v, "sending type");
    v = write(sd, userName, sizeof(userName)); error_check(v, "sending username");
    v = write(sd, passWord, sizeof(passWord)); error_check(v, "sending password");

    char resp;
    v = read(sd, &resp, 1); error_check(v, "Seeing if signup was valid");
    while(strcmp(&resp, BAD) == 0)
    {
      printf("There was a problem with your attempted registration.\n");
      printf("Input Username\n");
      v = fgets(userName, sizeof(userName), stdin); error_checkF(v, "line 107");
      nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
      printf("Input Password\n");
      fgets(passWord, sizeof(passWord), stdin); error_checkF(v, "line 110");
      nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

      v = write(sd, userName, sizeof(userName)); error_check(v, "line 113");
      v = write(sd, passWord, sizeof(passWord)); error_check(v, "line 114");
      v = read(sd, &resp, 1); error_check(v, "line 115");
    }

    printf("You are now signed up.\n");
  }
  else
  {
    write(sd, "l", 1);
    char userName[MAXMESSAGE]; bzero(userName, MAXMESSAGE);
    char passWord[MAXMESSAGE]; bzero(passWord, MAXMESSAGE);

    strcpy(userName, strtok(initialBuffer, " "));
    nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
    strcpy(passWord, strtok(initialBuffer, " "));
    nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

    v = write(sd, userName, sizeof(userName)); error_check(v, "line 131");
    v = write(sd, passWord, sizeof(passWord)); error_check(v, "line 132");

    char resp;
    v = read(sd, &resp, 1); error_check(v, "line 135");
    while(strcmp(&resp, BAD) == 0)
    {
      bzero(userName, MAXMESSAGE);
      bzero(passWord, MAXMESSAGE);

      printf("There was a problem with your attempted login.\n");
      printf("Input Username\n");
      v = fgets(userName, sizeof(userName), stdin); error_checkF(v, "line 143");
      nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
      printf("Input Password\n");
      v = fgets(passWord, sizeof(passWord), stdin); error_checkF(v, "line 146");
      nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

      v = write(sd, userName, sizeof(userName)); error_check(v, "line 149");
      v = write(sd, passWord, sizeof(passWord)); error_check(v, "line 150");
      v = read(sd, &resp, 1); error_check(v, "line 151");
    }

    printf("You are now logged in.\n");
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
      v = write(sd, request, sizeof(request)); error_check(v, "line 169");
      printf("Logged Out\n");
      exit(0);
    }
    else if(strcmp(command, "$gitProject -crf") == 0){
      v = write(sd, request, sizeof(request)); error_check(v, "line 174");
      char resp;
      read(sd, &resp, 1);
      if(strcmp(&resp, GOOD) == 0) printf("File created!\n");
      else printf("File not created.\n");
    }
    else if(strcmp(command, "$gitProject -edt") == 0){
      char text[MAXFILESIZE]; bzero(text, MAXFILESIZE);
      char returnText[MAXFILESIZE]; bzero(returnText, MAXFILESIZE);

      v = write(sd, request, MAXMESSAGE); error_check(v, "line 184");
      char resp;
      v = read(sd, &resp, 1); error_check(v, "line 186");

      char* fileName = request + COMMANDSIZE + 1;
      if(strcmp(&resp, BAD) == 0){
        printf("You cannot access %s. Either someone is editing it, or it is not shared with you. \n", fileName);
        continue;
      }
      v = read(sd, text, MAXFILESIZE); error_check(v, "line 193");

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
        v = execvp(cmd, argv); error_check(v, "line 207");
        return 0;
      }

      int status;
      v = wait(&status); error_check(v, "line 212");
      if(WIFEXITED(status)) {
        copyfile(fileName, returnText);
        v = write(sd, "$gitProject -rec", sizeof("$gitProject -rec")); error_check(v, "line 215");
        v = write(sd, " ", 1); error_check(v, "line 216");
        v = write(sd, fileName, sizeof(fileName)); error_check(v, "line 217");
        v = write(sd, returnText, sizeof(returnText)); error_check(v, "line 218");
        remove(fileName);
      }
      else{
        v = write(sd, "$gitProject -non", sizeof("$gitProject")); error_check(v, "line 222");
        remove(fileName);
      }
    }
    else if (strcmp(command, "$gitProject -rmf") == 0){
      v = write(sd, request, sizeof(request)); error_check(v, "line 227");
      char resp;
      v = read(sd, &resp, 1); error_check(v, "line 229");
      if(strcmp(&resp, GOOD) == 0) printf("File removed!\n");
      else printf("File not removed.\n");
    }
    else if(strcmp (command, "$gitProject -inv") == 0){
      v = write(sd, request, sizeof(request)); error_check(v, "line 234");
      char resp;
      v = read(sd, &resp, 1); error_check(v, "line 236");
      if(strcmp(&resp, GOOD) == 0) printf("Collaborator invited!\n");
      else printf("Collaborator not invited.\n");
    }
    else{
      printf("Request does not match any known command.\n");
    }
  }

  return 0;

}
