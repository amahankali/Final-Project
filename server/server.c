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

  printf("server was setup\n");
  return sd;
}

//creates a new socket connecting the server to a client
//we use this socket to send and receive file text from the server
//to this particular client
int server_connect(int sd) {
  int newsockfd, i;
  struct sockaddr_in sock1, cli_addr;

  i = listen(sd, 5);
  error_check( i, "server listen" );

  printf("about to accept\n");
  unsigned int sock1_len = sizeof(sock1);
  newsockfd = accept( sd, (struct sockaddr *)&sock1, &sock1_len );
  error_check( newsockfd, "server accept" );

  printf("[server] connected to %s\n", inet_ntoa( sock1.sin_addr ) );

  return newsockfd;
}

////////////////////////////////////////////////////////////////
//first COMMANDSIZE letters of command are a command
void getCommand(char* request, char* command){
  memcpy(command, request, COMMANDSIZE);
  command[COMMANDSIZE] = 0;
}

//copies contents of file named file into buffer
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

//given a file name, it creates a name with same name, but
//ending in .jfk - this new file will contain the list of users
//allowed to see the file
//assumes filename ends in '.txt'
char* permFile(char* filename){
  char* ans = calloc(1, MAXMESSAGE + 20);
  strcpy(filename, ans);
  ans = strsep(&ans, ".");
  strcat(ans, ".jfk");
  touch(ans);
  return ans;
}

//check if user can edit file
int validateUser(char* filename, char* username){
    char buffer[MAXFILESIZE];
    char* permfile = permFile(filename);
    copyfile(permfile, buffer);
    char* name;
    while((name = strsep(&buffer,"\n")) != NULL){
      if (strcmp(name, username) == 0) return 1;
    }
    return 0;
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
        close(newsockfd);
        continue;
      }

      //////////////////Logging in - registers user if needed//////////////////
      //Note: The first communication will always have to be the login/signup.
      char type;
      read(newsockfd, &type, 1);

      char username[MAXMESSAGE + 1]; bzero(username, MAXMESSAGE + 1);
      char password[MAXMESSAGE + 1]; bzero(password, MAXMESSAGE + 1);

      read(newsockfd, username, MAXMESSAGE); //double check if it blocks with sockets
      read(newsockfd, password, MAXMESSAGE);

      int c;
      if(type == 'r') c = signUp(username, password);
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

          if(strcmp(commandType, "$gitProject -lgo") == 0) exit(0); //logging out
          else if(strcmp(commandType, "$gitProject -crf") == 0)
          {
            //the client is asking to create a file
            char* filename = request + COMMANDSIZE + 1;
            int c = touch(filename); //+1 is because of space

            if(!c)
            {
              write(newsockfd, BAD, 1);
              continue; //prompt for another command on client-side
            }

            //setup bookkeeping: permission file, semaphore
            char* permfile = permFile(filename);
            int permFD = open(permfile, O_WRONLY);
            write(permFD, username, strlen(username));
            write(permFD, "\n", 5);
            close(permFD);
            free(permfile);

            //semaphore
            int key = ftok(filename, 12);
            int semd = semget(key, 1, IPC_CREAT | 0644);
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = 1;
            semop(semd, &op, 1);

            write(newsockfd, GOOD, 1);
          }
          else if(strcmp(commandType, "$gitProject -edt") == 0)
          {
            //client is asking to open a file
            //server should send updated version
            char* filename = request + COMMANDSIZE + 1;

            //check if he is allowed
            validateUser(filename, username);

            //check semaphore
            int key = ftok(filename, 12);
            int semd = semget(key, 1, 0644);
            int val = semctl(semd, 0, GETVAL);
            if(!val)
            {
              write(newsockfd, BAD, 1);
              continue; //prompt for another command on client-side
            }

            //update semaphore - down
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = -1;
            semop(semd, &op, 1);

            //send contents of file to client
            char* filebuf = (char *) calloc(1, MAXFILESIZE + 1);
            int fd = open(filename, O_RDONLY);
            read(fd, filebuf, MAXFILESIZE);
            close(fd);
            write(newsockfd, GOOD, 1);
            write(newsockfd, filebuf, MAXFILESIZE);
          }
          else if(strcmp(commandType, "$gitProject -rec") == 0)
          {
            //this is not a command, but this is the message
            //the client will send once the user is done editing a file
            char* filename = request + COMMANDSIZE + 1;

            //get contents of file and replace contents of file
            char* filebuf = (char *) calloc(1, MAXFILESIZE + 1);
            int fd = open(filename, O_TRUNC | O_WRONLY);
            read(newsockfd, filebuf, MAXFILESIZE);
            write(fd, filebuf, MAXFILESIZE);
            close(fd);

            //update semaphore - up
            int key = ftok(filename, 12);
            int semd = semget(key, 1, 0644);
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = 1;
            semop(semd, &op, 1);
          }
          else if(strcmp(commandType, "$gitProject -rmf") == 0)
          {
            //the client is asking to remove a file
            char* filename = request + COMMANDSIZE + 1;
            int err = open(filename, O_CREAT | O_EXCL);
            if(!err)
            {
              write(newsockfd, BAD, 1);
              continue;
            }

            //check semaphore
            int key = ftok(filename, 12);
            int semd = semget(key, 1, 0644);
            int val = semctl(semd, 0, GETVAL);
            if(!val)
            {
              write(newsockfd, BAD, 1);
              continue; //prompt for another command on client-side
            }

            //remove semaphore
            semctl(semd, 0, IPC_RMID);

            //remove perm file
            char* permfile = permFile(filename);
            remove(permfile);

            //remove file
            remove(filename);

          }
          else if(strcmp(commandType, "$gitProject -inv") == 0)
          {
            //the client is asking to share a file with
            //another user

            char* filename = request + COMMANDSIZE + 1;
            int err = open(filename, O_CREAT | O_EXCL); //if file does not exist, it cannot be shared
            if(!err)
            {
              write(newsockfd, BAD, 1);
              continue;
            }

            ///////////////////////////////////////////////////////////////////////
            //check if the user is an owner of the file (you have to be owner to invite) -
            //in the permFile, the owner will always be first line
            char* permfile = permFile(filename);
            //get first line of permfile. this should be the user
            char* owner = calloc(1, MAXFILESIZE + 1);
            int permFD = open(permfile, O_RDONLY);
            read(permFD, owner, MAXFILESIZE);
            close(permFD);

            owner = strsep(&owner, "\n");
            if(strcmp(username, owner)) //user is not the onwer
            {
              write(newsockfd, BAD, 1);
              free(owner);
              continue;
            }
            free(owner);
            ///////////////////////////////////////////////////////////////////////


            //add the other user to the permfile of this file
            char* otheruser = filename + strlen(filename) + 1;
            permFD = open(permfile, O_APPEND);
            write(permFD, otheruser, MAXMESSAGE);
            close(permFD);

          }
          else write(newsockfd, BAD, 1);

          /*
          if (strcmp(subbuff, "$gitProject -c") == 0){ //creates new file
            char* file;
            char filename[64];
            memcpy( filename, &buffer[15], sizeof(buffer) );
            touch(filename);
            textfile(filename); //creates text file that we can insert permissions
          }
          if (strcmp(subbuff, "$gitProject -e") == 0){//send file to client
            char* file;
            char filename[64];
            memcpy( filename, &buffer[15], sizeof(buffer) );
            copyfile(filename, file);
            write(newsockfd, file, strlen(file));
          }
          if (strcmp(subbuff, "$gitProject -r") == 0){//receives file from client and saves it on sever
            char* fileText;
            char filename[64];
            memcpy( filename, &buffer[15], sizeof(buffer) );
            char file[MAXFILESIZE];
            read(newsockfd,fileText,sizeof(fileText));
            writeFile(fileText, filename);
          }
          */


      }




  }

  return 0;

}
