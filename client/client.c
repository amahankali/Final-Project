#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
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
void error_checkF(char* i, char *s ) {
  if(i <= 0) {
    printf("%lu\n", (unsigned long) i);
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
  int fd = open(file, O_RDONLY, 0666);
  read(fd, buffer, MAXMESSAGE);
  close(fd);
}

//replaces contents of file named file with buffer
void writeFile(char* buffer, char* file)
{
  int fd = open(file, O_TRUNC | O_WRONLY, 0666); //double check
  write(fd, buffer, strlen(buffer));
  close(fd);
}

int touch(char* filename)
{
  int fd = open(filename, O_CREAT, 0666);
  close(fd);
  return 1;
}

///////////////Communication with Sockets///////////////
//ENSURES THAT CLIENT AND SERVER STAY IN SYNC
//THIS WAS THE PROBLEM ALL ALONG

//all the buffers are null terminated

//reads a message from sd of length MAXMESSAGE,
//which is padded by #, into receiving, which then
//replaces all # with /0
//in the beginning, receiving is full of zeros
//in the end, receiving is a normal string consisting
//of the mssage, null-terminated
//cannot use with string literals
void aReadAux(int sd, char* receiving)
{
  read(sd, receiving, MAXMESSAGE);
  char* firstZero = strchr(receiving, '#');
  if(firstZero)
  {
    int zeroLength = (MAXMESSAGE + 1) - (firstZero - receiving); //length of buffer minus length of actual message
    bzero(firstZero, zeroLength);
  }
}

void aRead(int sd, char* str)
{
  char receiving[MAXMESSAGE + 1];
  bzero(receiving, MAXMESSAGE + 1);
  aReadAux(sd, receiving);
  strcpy(str, receiving);
}

//sending is a char array with all zeros
//no side effects of aWrite
//cannot use with string literals
void aWriteAux(int sd, char* sending)
{
  char* firstZero = sending + strlen(sending); char* originalFirstZero = firstZero;
  while(firstZero < sending + MAXMESSAGE) *(firstZero++) = '#'; //sending[MAXMESSAGE] is left '\0'
  write(sd, sending, MAXMESSAGE);

  //restore message to original in case it is used again
  firstZero = originalFirstZero;
  while(firstZero < sending + MAXMESSAGE)
  {
    *firstZero = '\0';
    firstZero++;
  }
}

void aWrite(int sd, char* str)
{
  char sending[MAXMESSAGE + 1];
  bzero(sending, MAXMESSAGE + 1);
  strcpy(sending, str);
  aWriteAux(sd, sending);
}

////////////////////////////////////////////////////////
/*
int main () {

  int sd = client_connect(TESTIP, TESTPORT);
  char receiving[MAXMESSAGE + 1]; bzero(receiving, MAXMESSAGE + 1);
  char sending[MAXMESSAGE + 1]; bzero(sending, MAXMESSAGE + 1);

  aRead(sd, receiving);

  printf("Received the message: \n");
  printf("%s\n", receiving);

  strcpy(sending, receiving);
  aWrite(sd, "Received the message: \n");
  aWrite(sd, sending);

  printf("Wrote both messages\n");

  close(sd);

}
*/


int main () {
  umask(0000);
  char initialBuffer[256];

  int sd = client_connect(TESTIP, TESTPORT);

  printf("Running gitProject\n If you wish to sign up please hit enter without typing.\n");
  printf("Otherwise, enter your username and password on one line, separated by a space.\n");
  int v;
  char* v2 = fgets(initialBuffer, sizeof(initialBuffer), stdin); error_checkF(v2, "Reading first communication");
  char* nLine = strchr(initialBuffer, '\n'); if(nLine) *nLine = '\0'; //remove new line in initialBuffer

  //////////////////Logging in - registers user if needed//////////////////
  if (initialBuffer[0] == '\0')
  {
    char userName[MAXMESSAGE]; bzero(userName, MAXMESSAGE);
    char passWord[MAXMESSAGE]; bzero(passWord, MAXMESSAGE);

    //first reading of username and password
    printf("Input Username\n");
    v2 = fgets(userName, sizeof(userName), stdin); error_checkF(v2, "line 90");
    nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
    printf("Input Password\n");
    v2 = fgets(passWord, sizeof(passWord), stdin); error_checkF(v2, "line 93");
    nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';
    /////////////

    aWrite(sd, "r");
    aWrite(sd, userName);
    aWrite(sd, passWord);

    char resp[2]; resp[0] = resp[1] = '\0';
    aRead(sd, resp);
    while(strcmp(resp, BAD) == 0)
    {
      printf("There was a problem with your attempted registration.\n");
      printf("Input Username\n");
      v2 = fgets(userName, sizeof(userName), stdin); error_checkF(v2, "line 107");
      nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
      printf("Input Password\n");
      v2 = fgets(passWord, sizeof(passWord), stdin); error_checkF(v2, "line 110");
      nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

      aWrite(sd, userName);
      aWrite(sd, passWord);
      aRead(sd, resp);
    }

    printf("You are now signed up.\n");
  }
  else
  {
    aWrite(sd, "l");
    char userName[MAXMESSAGE]; bzero(userName, MAXMESSAGE);
    char passWord[MAXMESSAGE]; bzero(passWord, MAXMESSAGE);

    strcpy(userName, strtok(initialBuffer, " "));
    nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
    strcpy(passWord, strtok(initialBuffer, " "));
    nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

    aWrite(sd, userName);
    aWrite(sd, passWord);

    char resp[2]; resp[0] = resp[1] = '\0';
    aRead(sd, resp);
    while(strcmp(resp, BAD) == 0)
    {
      bzero(userName, MAXMESSAGE);
      bzero(passWord, MAXMESSAGE);

      printf("There was a problem with your attempted login.\n");
      printf("Input Username\n");
      v2 = fgets(userName, sizeof(userName), stdin); error_checkF(v2, "line 143");
      nLine = strchr(userName, '\n'); if(nLine) *nLine = '\0';
      printf("Input Password\n");
      v2 = fgets(passWord, sizeof(passWord), stdin); error_checkF(v2, "line 146");
      nLine = strchr(passWord, '\n'); if(nLine) *nLine = '\0';

      aWrite(sd, userName);
      aWrite(sd, passWord);
      aRead(sd, resp);
    }

    printf("You are now logged in.\n");
  }
  /////////////////////////////////////////////////////////////////////////

  while(1)
  {
    printf("===================================================================\n");

    char request[MAXMESSAGE + 1]; bzero(request, MAXMESSAGE + 1);
    char command[COMMANDSIZE + 1]; bzero(command, COMMANDSIZE + 1);

    v2 = fgets(request, sizeof(request), stdin); error_checkF(v2, "line 164.");
    nLine = strchr(request, '\n'); if(nLine) *nLine = '\0';
    memcpy(command, &request, COMMANDSIZE);
    command[COMMANDSIZE] = '\0';

    printf("Your request was: %s.\n", request);
    printf("Your command was: %s.\n", command);

    char fileName[MAXMESSAGE];
    if(strcmp(request, "$gitProject -lgo") == 0){
      aWrite(sd, request);
      printf("Logged Out\n");
      exit(0);
    }
    else if(strcmp(command, "$gitProject -crf") == 0){
      aWrite(sd, request);
      char resp[2]; resp[0] = resp[1] = '\0';
      aRead(sd, resp);
      if(strcmp(resp, GOOD) == 0) printf("File created!\n");
      else printf("File not created.\n");
    }
    else if(strcmp(command, "$gitProject -edt") == 0){
      char text[MAXMESSAGE]; bzero(text, MAXMESSAGE);
      char returnText[MAXMESSAGE]; bzero(returnText, MAXMESSAGE);

      aWrite(sd, request);
      char resp[2]; resp[0] = resp[1] = '\0'; printf("Initial Response Buffer: %s\n", resp);
      aRead(sd, resp);

      char* fileName = request + COMMANDSIZE + 1;
      if(strcmp(resp, BAD) == 0){
        printf("You cannot access %s. Either someone is editing it, or it is not shared with you. \n", fileName);
        continue;
      }
      else if(strcmp(resp, GOOD) == 0){
        aRead(sd, text);

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
        printf("Finished running emacs\n");

        int status;
        v = wait(&status); error_check(v, "line 212");
        if(WIFEXITED(status)) {
          copyfile(fileName, returnText);
          printf("Changing the file\n");
          printf("New text: \n");
          printf("%s\n", returnText);

          char message[MAXMESSAGE + 1]; bzero(message, MAXMESSAGE + 1);
          strcat(message, "$gitProject -rec "); strcat(message, fileName);
          aWrite(sd, message);
          aWrite(sd, returnText);
          remove(fileName);
        }
        else{
          printf("Not changing the file\n");

          char message[MAXMESSAGE + 1]; bzero(message, MAXMESSAGE + 1);
          strcat(message, "$gitProject -non "); strcat(message, fileName);
          aWrite(sd, message);
          remove(fileName);
        }
      }
      else {
        printf("No response from server on whether file can be edited or not\n");
      }
    }
    else if (strcmp(command, "$gitProject -rmf") == 0){
      aWrite(sd, request);
      char resp[2]; resp[0] = resp[1] = '\0';
      aRead(sd, resp);
      if(strcmp(resp, GOOD) == 0) printf("File removed!\n");
      else printf("File not removed.\n");
    }
    else if(strcmp (command, "$gitProject -inv") == 0){
      aWrite(sd, request);
      char resp[2]; resp[0] = resp[1] = '\0';
      aRead(sd, resp);
      if(strcmp(resp, GOOD) == 0) printf("Collaborator invited!\n");
      else printf("Collaborator not invited.\n");
    }
    else{
      printf("Request does not match any known command.\n");
    }

    printf("===================================================================\n");
  }

  return 0;

}
