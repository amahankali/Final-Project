#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "constant.h"
#include "auxServerFunc.h"

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
  int fd = open(file, O_RDONLY, 0666);
  read(fd, buffer, MAXFILESIZE);
  close(fd);
}

//replaces contents of file named file with buffer
void writeFile(char* buffer, char* file)
{
  int fd = open(file, O_TRUNC | O_WRONLY, 0666); //double check
  write(fd, buffer, strlen(buffer));
  close(fd);
}

//given a file name, it creates a name with same name, but
//ending in .jfk - this new file will contain the list of users
//allowed to see the file
//assumes filename ends in '.txt'
char* permFile(char* filename){
  char* ans = calloc(1, MAXMESSAGE + 20);
  strcpy(ans, filename);
  ans = strsep(&ans, ".");
  strcat(ans, ".jfk");
  touch(ans);
  printf("permfile of %s: %s\n", filename, ans);
  return ans;
}

//check if user can edit file
int validateUser(char* filename, char* username){
    char buffer[MAXFILESIZE];
    char* permfile = permFile(filename);
    copyfile(permfile, buffer);
    free(permfile);
    char* name;
    while((name = strsep(&buffer,"\n")) != NULL){
      if (strcmp(name, username) == 0) return 1;
    }
    return 0;
}

int main() {
  umask(0000);
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
      int v = read(newsockfd, &type, 1); error_check(v, "Type of sign-in");

      char username[MAXMESSAGE + 1]; bzero(username, MAXMESSAGE + 1);
      char password[MAXMESSAGE + 1]; bzero(password, MAXMESSAGE + 1);

      v = read(newsockfd, username, MAXMESSAGE); error_check(v, "Reading username");
      v = read(newsockfd, password, MAXMESSAGE); error_check(v, "Reading password");
      printf("Given username: %s\n", username);
      printf("Given password: %s\n", password);

      int c;
      if(type == 'r') c = signUp(username, password);
      else c = login(username, password);
      while(!c) //these functions return 0 on failure
      {
        v = write(newsockfd, BAD, 1); error_check(v, "Sending response: was sign-in valid?");
        v = read(newsockfd, username, MAXMESSAGE); error_check(v, "Reading username again");
        v = read(newsockfd, password, MAXMESSAGE); error_check(v, "Reading password again");
        if(type == 'r') c = signUp(username, password);
        else c = login(username, password);
      }
      v = write(newsockfd, GOOD, 1); error_check(v, "Response confirming login");
      /////////////////////////////////////////////////////////////////////////


      char request[MAXMESSAGE + 1]; //this buffer is used to receive requests
      bzero(request, MAXMESSAGE + 1);

      char commandType[COMMANDSIZE + 1];
      bzero(commandType, COMMANDSIZE + 1);

      while(1)
      {
          printf("===================================================================\n");

          bzero(request, MAXMESSAGE + 1);
          bzero(commandType, MAXMESSAGE + 1);

          v = read(newsockfd, request, MAXMESSAGE); error_check(v, "Getting client request");
          getCommand(request, commandType);

          printf("Request received: %s\n", request);
          printf("Command received: %s\n", commandType);

          if(strcmp(commandType, "$gitProject -lgo") == 0) exit(0); //logging out
          else if(strcmp(commandType, "$gitProject -crf") == 0)
          {
            printf("Creating a file\n");
            //the client is asking to create a file
            char* filename = request + COMMANDSIZE + 1;
            int c = touch(filename); //+1 is because of space
            printf("Filename being created: %s\n", filename);

            if(!c)
            {
              v = write(newsockfd, BAD, 1); error_check(v, "Telling that file could not be created");
              continue; //prompt for another command on client-side
            }

            //setup bookkeeping: permission file, semaphore
            char* permfile = permFile(filename);
            printf("Permfile of current file: %s\n", permfile);
            int permFD = open(permfile, O_WRONLY, 0666); error_check(permFD, "Opening file for permission checking");
            v = write(permFD, username, strlen(username)); error_check(v, "Setting ownership");
            v = write(permFD, "\n", 5); error_check(v, "new line");
            v = close(permFD); error_check(v, "closing permission file");
            free(permfile);

            //semaphore
            int key = ftok(filename, 12); error_check(key, "Making key to create semaphore");
            int semd = semget(key, 1, IPC_CREAT | 0644); error_check(semd, "Creating semaphore for new file");
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = 1;
            v = semop(semd, &op, 1); error_check(v, "Initializing semaphore for new file");

            v = write(newsockfd, GOOD, 1); error_check(v, "Confirming creation of new file");
          }
          else if(strcmp(commandType, "$gitProject -edt") == 0)
          {
            printf("editing a file\n");
            //client is asking to open a file
            //server should send updated version
            char* filename = request + COMMANDSIZE + 1;
            //check if he is allowed
            validateUser(filename, username);

            //check semaphore
            int key = ftok(filename, 12); error_check(key, "Getting key to edit file");
            int semd = semget(key, 1, 0644); error_check(semd, "Opening semaphore of file");
            int val = semctl(semd, 0, GETVAL); error_check(val, "Value of semaphore should be nonnegative");
            if(val < 0) printf("Value of semaphore was negative\n");
            if(!val)
            {
              v = write(newsockfd, BAD, 1); error_check(v, "Telling that file could not be opened");
              continue; //prompt for another command on client-side
            }

            //update semaphore - down
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = -1;
            v = semop(semd, &op, 1); error_check(v, "Marking file as editing with semaphore");

            //send contents of file to client
            char* filebuf = (char *) calloc(1, MAXFILESIZE + 1);
            int fd = open(filename, O_RDONLY, 0666); error_check(fd, "Opening file to prepare for copying");
            v = read(fd, filebuf, MAXFILESIZE); error_check(v, "Getting contents of file");
            v = close(fd); error_check(v, "Done using file for now");
            v = write(newsockfd, GOOD, 1); error_check(v, "Confirming file accessible");
            v = write(newsockfd, filebuf, MAXFILESIZE); error_check(v, "Sending file over socket");
            free(filebuf);
          }
          else if(strcmp(commandType, "$gitProject -rec") == 0)
          {
            printf("receiving a file\n");
            //this is not a command, but this is the message
            //the client will send once the user is done editing a file
            char* filename = request + COMMANDSIZE + 1;

            //get contents of file and replace contents of file
            char* filebuf = (char *) calloc(1, MAXFILESIZE + 1);
            int fd = open(filename, O_TRUNC | O_WRONLY, 0666); error_check(fd, "Open file for copying");
            v = read(newsockfd, filebuf, MAXFILESIZE); error_check(v, "Get new contents from socket");
            v = write(fd, filebuf, MAXFILESIZE); error_check(v, "Write new contents into file");
            v = close(fd); error_check(v, "close file");
            free(filebuf);

            //update semaphore - up
            int key = ftok(filename, 12); error_check(key, "Getting key to receive file");
            int semd = semget(key, 1, 0644); error_check(semd, "Access semaphore - change made");
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = 1;
            v = semop(semd, &op, 1); error_check(v, "Allow other users to access file - increment semaphore - change made");
          }
          else if(strcmp(commandType, "$gitProject -non") == 0)
          {
            printf("no changes made to file\n");
            //this is not a command, but this is the message
            //the client will send once the user tried to edit a file
            //and there was an error
            char* filename = request + COMMANDSIZE + 1;

            //update semaphore - up
            int key = ftok(filename, 12); error_check(key, "Getting key to allow access to file");
            int semd = semget(key, 1, 0644); error_check(semd, "Access semaphore - no change made");
            struct sembuf op;
            op.sem_num = 0;
            op.sem_op = 1;
            v = semop(semd, &op, 1); error_check(v, "Allow other users to access file - increment semaphore - no change made");
          }
          else if(strcmp(commandType, "$gitProject -rmf") == 0)
          {
            //the client is asking to remove a file
            char* filename = request + COMMANDSIZE + 1;

            //check semaphore
            int key = ftok(filename, 12); error_check(key, "Getting key for semaphore");
            int semd = semget(key, 1, 0644); error_check(semd, "Access semaphore");
            int val = semctl(semd, 0, GETVAL); error_check(val, "Getting value of semaphore");
            if(!val)
            {
              v = write(newsockfd, BAD, 1); error_check(v, "Telling that file could not be removed");
              continue; //prompt for another command on client-side
            }

            //remove semaphore
            v = semctl(semd, 0, IPC_RMID); error_check(v, "Semaphore could not be removed");

            //remove perm file
            char* permfile = permFile(filename);
            remove(permfile);
            free(permfile);

            //remove file
            remove(filename);

          }
          else if(strcmp(commandType, "$gitProject -inv") == 0)
          {
            //the client is asking to share a file with
            //another user
            printf("sharing a file\n");
            char* filename = request + COMMANDSIZE + 1;

            ///////////////////////////////////////////////////////////////////////
            //check if the user is an owner of the file (you have to be owner to invite) -
            //in the permFile, the owner will always be first line
            char* permfile = permFile(filename);
            //get first line of permfile. this should be the user
            char* owner = calloc(1, MAXFILESIZE + 1);
            int permFD = open(permfile, O_RDONLY, 0666); error_check(permFD, "getting permission file");
            v = read(permFD, owner, MAXFILESIZE); error_check(v, "getting string containing name of owner");
            close(permFD);

            owner = strsep(&owner, "\n");
            if(strcmp(username, owner)) //user is not the onwer
            {
              v = write(newsockfd, BAD, 1); error_check(v, "Telling that user is not owner, so cannot invite");
              free(owner);
              free(permfile);
              continue;
            }
            free(owner);
            ///////////////////////////////////////////////////////////////////////


            //add the other user to the permfile of this file
            char* otheruser = filename + strlen(filename) + 1;
            permFD = open(permfile, O_APPEND, 0666);
            v = write(permFD, otheruser, MAXMESSAGE); error_check(v, "Adding user to permissions file");
            v = close(permFD); error_check(v, "closing permissions file");
            free(permfile);

          }
          else write(newsockfd, BAD, 1);

          printf("===================================================================\n");
      }




  }

  return 0;

}
