#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "constant.h"
#include "auxServerFunc.h"

void error_check( int i, char *s ) {
  if ( i < 0 ) {
    printf("%d\n", i);
    printf("[%s] error %d: %s\n", s, errno, strerror(errno) );
    exit(1);
  }
}

////////////////////////CONNECTION SETUP////////////////////////

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

  i = listen(sd, 0);
  error_check( i, "server listen" );

  unsigned int sock1_len = sizeof(sock1);
  newsockfd = accept( sd, (struct sockaddr *)&sock1, &sock1_len );
  error_check( newsockfd, "server accept" );

  printf("[server] connected to %s\n", inet_ntoa( sock1.sin_addr ) );

  return newsockfd;
}

////////////////////////////////////////////////////////////////

void getCommand(char* request, char* command){
  memcpy(command, request, COMMANDSIZE);
  command[COMMANDSIZE] = 0;
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

void textFile(char* buffer){
  fileName = strsep(&buffer, '.');
  strcat(fileName, ".jfk");
  touch(fileName);
}

int validateUser(char* fileName, char* userName){
    char buffer[MAXFILESIZE], *name;
    copyfile(fileName, buffer);
    while( (name = strsep(buffer,"\n")) != NULL ){
      if (name == userName)
        return 1;
    }
    return -1;
}

//char* filecopy(char buffer){}


/*char savefile(char name){
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
*/

int main() {

  int mainSD = server_setup(TESTPORT);
  while(1)
  {
      int newsockfd = server_connect(mainSD);

      int serverF = fork();
      if(serverF)
      {
        close(branch);
        continue;
      }

      //////////////////Logging in - registers user if needed//////////////////
      //Note: The first communication will always have to be the login/signup.
      char* type = (char *) calloc(1, 1);
      read(newsockfd, type, 1);

      char* username = (char *) calloc(1, MAXMESSAGE + 1); bzero(username, MAXMESSAGE + 1);
      char* password = (char *) calloc(1, MAXMESSAGE + 1); bzero(password, MAXMESSAGE + 1);

      read(newsockfd, username, MAXMESSAGE); //double check if it blocks with sockets
      read(newsockfd, password, MAXMESSAGE);

      int c;
      if(*type == 'r') c = signUp(username, password);
      else c = login(username, password);
      while(!c) //these functions return 0 on failure
      {
        write(newsockfd, BAD, 1);
        read(newsockfd, username, MAXMESSAGE); //double check if it blocks with sockets
        read(newsockfd, password, MAXMESSAGE);
        if(type == 'r') c = signUp(username, password);
        else c = login(username, password);
      }
      write(newsockfd, GOOD, 1);
      /////////////////////////////////////////////////////////////////////////


      char request[MAXMESSAGE + 1]; //this buffer is used to receive requests
      bzero(request, MAXMESSAGE + 1);

      char commandType[COMMANDSIZE + 1];
      bzero(commandType, COMMANDSIZE + 1);

      while(1)
      {
          read(newsockfd, request, MAXMESSAGE);
          getCommand(request, commandType);

          /*
          Types of commands and their symbols
          1. $gitProject invite <FILE/DIRECTORY> <USER> <PERMISSIONS>: allow USER to see FILE/DIRECTORY and all of its contents
          if he chooses the second option, and do other things depending on PERMISSIONS
          2. $gitProject createFile <FILE>: creates file with name FILE in current directory
          3. $gitProject createFolder <FOLDER>: creates folder with name FOLDER in current directory
          4. $gitProject deleteFile <FILE>: delete file with name FILE in current directory if it exists
          5. $gitProject deleteFolder <FOLDER>
          6. $gitProject open <FILE>: opens FILE in user's choice of editor
          7. $gitProject logout

          8. We are planning to implement gcc and execute - only c code can be run using our thing
          */

          if(strcmp(commandType, "$gitProject -lgo") == 0) exit(0); //logging out
          if(strcmp(commandType, "$gitProject -crf") == 0)
          {
            //the client is asking to create a file
            char* fileName = request + COMMANDSIZE; //relative path from root of server file system to requested file
            int c = touch(fileName);

            if(!c)
            {
              write(newsockfd, BAD, 1);
              continue; //prompt for another command on client-side
            }

            //setup bookkeeping: permission file, semaphore
            textfile(fileName);

            //semaphore
            int key = ftok(fileName, 12);
            int semd = semget(key, 1, IPC_CREAT | 0644);
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = 1;
            semop(semd, op, 1);

            write(newsockfd, GOOD, 1);
          }
          if(strcmp(commandType, "$gitProject -crd") == 0)
          {
            //the client is asking to create a directory
          }
          if(strcmp(commandType, "$gitProject -edt") == 0)
          {
            //client is asking to open a file
            //server should send updated version
          }
          if(strcmp(commandType, "$gitProject -rec") == 0)
          {
            //this is not really a command, but this is the message
            //the client will send once the user is done editing a file
          }
          if(strcmp(commandType, "$gitProject -rmf") == 0)
          {
            //the client is asking to remove a file
          }
          if(strcmp(commandType, "$gitProject -rmd") == 0)
          {
            //the client is asking to remove a directory
          }
          if(strcmp(commandType, "$gitProject -inv") == 0)
          {
            //the client is asking to share a file with
            //another user
          }
          if(strcmp(commandType, "$gitProject -gcc") == 0)
          {
            //the client is asking to compile a file
          }
          if(strcmp(commandType, "$gitProject -exe") == 0)
          {
            //the client is asking to execute a file
          }


          /*
          if (strcmp(subbuff, "$gitProject -c") == 0){ //creates new file
            char* file;
            char fileName[64];
            memcpy( fileName, &buffer[15], sizeof(buffer) );
            touch(fileName);
            textfile(fileName); //creates text file that we can insert permissions
          }
          if (strcmp(subbuff, "$gitProject -e") == 0){//send file to client
            char* file;
            char fileName[64];
            memcpy( fileName, &buffer[15], sizeof(buffer) );
            copyfile(fileName, file);
            write(newsockfd, file, strlen(file));
          }
          if (strcmp(subbuff, "$gitProject -r") == 0){//receives file from client and saves it on sever
            char* fileText;
            char fileName[64];
            memcpy( fileName, &buffer[15], sizeof(buffer) );
            char file[MAXFILESIZE];
            read(newsockfd,fileText,sizeof(fileText));
            writeFile(fileText, fileName);
          }
          */
      }




  }

  return 0;

}
