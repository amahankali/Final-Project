#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>

#define MAXCOMMANDS 50

/*=======================================================
void print(char* a[], int length): prints out the contents
of a given array
    - Parameters:
        - a is the array
        - length is the number of elements in a
    - Returns:
        - nothing
=======================================================*/
void print(char* a[], int length)
{
  int i = 0;
  for(; i < length; i++) printf("a[%d]: %s; ", i, a[i]);
  printf("\n");
}

int piper(char *a);

/*======================================================
int run(char *a):
- Tests for possible exceptions including echo to see if
it should decrease white space size or not. It then parses
the string removing consecutive whitespaces and whitespaces
at the beginning and end. After this it tests the exceptions
cd and cd ~ to know if it should go back to the home directory
and the exit function. If none of these have occurred it will
execvp convert the parsed string to an array and execvp
it.
-Returns an integer
-a is the user input string
======================================================*/
int run(char* a) {
  char *ans[50];
  char *n = calloc(1, strlen(a));
  int i=0;
  int j=0;
    if((*a == '#' && *(a+1) == '#' && *(a + 2)=='#' ) || *a == 0){return 1;}
    if((*a == 'e' || *a == 'E') && (*(a + 1) == 'c' || *(a + 1) == 'C')
       && (*(a + 2) == 'h' || *(a + 2) == 'H') && (*(a + 3) == 'o' || *(a + 3) == 'O')){
        strsep(&a, " ");
        printf("%s\n",a);
        return 1;
    }
  while(*(a+i)){
    if ((*(a + i) == ' ' && j == 0) ||
      (i!=0 && *(a + i - 1) == ' ' && *(a + i) == ' ')){
          i++;
          continue;
      }
      else{
          *(n+j) = *(a+i);
          i++;
          j++;
      }
    }

    if(j > 0 && *(n+j - 1)  == ' '){
      //printf("s");
      *(n+j-1) = 0;
    }
    i=0;
    while(*(n+i)){
        if(*(n+i) == '|'){
            return piper(n);
        }
        i++;
    }
    i = 0;
    while(n){
      char *s;
      s = strsep(&n, " ");
      ans[i] = s;
      //printf("%s %s\n", s, ans[i]);
      i++;
    }

    ans[i] = 0;

    if(strcmp(ans[0], "cd") == 0)
    {
        //printf("%s\n", ans[1]);
        if(ans[1] == NULL || !strcmp(ans[1],"~")){
            chdir(getenv("HOME"));
        }
        else if(chdir(ans[1])!=0){
            fprintf(stderr, "Error: No such directory\n");
        }
        return 1;
    }
    if(strcmp(ans[0], "exit") == 0)
    {
        exit(0);
    }

    ///////////////////////////Do redirection///////////////////////////
    //case of ">" or "<" at end
    if(strcmp(ans[i - 1], ">") == 0 || strcmp(ans[i - 1], "<") == 0)
    {
      fprintf(stderr, "-bash: syntax error near unexpected token `newline'\n");
      return 1;
    }


    int f = fork();
    int status;
    if(f == 0)
    {
      char* buffer[MAXCOMMANDS];
      int j = 0;
      int bufferLen = 0;

      for(; j < i;)
      {
        char* token = ans[j];
        if(strcmp(token, ">") == 0)
        {
          char* outFile = ans[j + 1];
          int newFD = open(outFile, O_TRUNC | O_WRONLY | O_RDONLY | O_CREAT, 0644);
          if(newFD == -1)
          {
            fprintf(stderr, "-bash: error: %s\n", strerror(errno));
            kill(getpid(), SIGTERM);
            //exit(0);
          }
          dup2(newFD, 1);
          j += 2;
        }
        else if(strcmp(token, "<") == 0)
        {
          char* inFile = ans[j + 1];
          int newFD = open(inFile, O_RDONLY);
          if(newFD == -1)
          {
            fprintf(stderr, "-bash: %s: %s\n", inFile, strerror(errno));
            kill(getpid(), SIGTERM);
            //exit(0);
          }
          dup2(newFD, 0);
          j += 2;
        }
        else
        {
          buffer[bufferLen] = ans[j];
          bufferLen++;
          j++;
        }
      }
      buffer[bufferLen] = 0;
      char subbuff[15];
      memcpy( subbuff, &buffer, 14 );
      subbuff[15] = '\0';
      if(strcmp(buffer, "$gitProject -e") == 0){
        clientEdit();
      }
      else if (strcmp(buffer, "$gitProject -s") == 0){
        clientSend();
      }
      else
        int ret = execvp(buffer[0], buffer);
      if(ret)
      {
        fprintf(stderr, "-bash: %s: command not found\n", buffer[0]);
        exit(0);
          return 1;
      }
      exit(0);
    }
    else
    {
      int reply = wait(&status);
      if(WIFSIGNALED(status)) kill(getpid(), SIGTERM);
    }
    ////////////////////////////////////////////////////////////////////

    return 1;


}

/*=======================================================
int piper(char *a): Parses the given string by the '|'
character and proceeds to check for notable exceptions.
If there are none then the program forks, redirects in the
child process and reverts back in the parent process.
    - Parameters:
        - a, a string to parse
    - Returns:
        - An integer to terminate the function
=======================================================*/
int piper(char *a){
    //deal with case of | at ending
    //testing

    if(a[strlen(a) - 1] == '|') //checks if last character is '|'
    {
      //read the last command from stdin
      char* aux = (char *) calloc(1, 50);

      //printf("\n");
      printf("> ");

      fgets(aux, 50, stdin); if(aux[strlen(aux) - 1] == '\n') aux[strlen(aux) - 1] = 0;

      strcat(a, " ");
      strcat(a, aux);
      free(aux);

      //printf("There was an ending '|'. The new command will be: %s.\n", a);

      return piper(a);
    }

    char *second = (char *)malloc(256);
    second = strsep(&a, "|");

    int arr[2];
    pipe(arr);
    //int out = dup(STDOUT_FILENO);
    int in = dup(STDIN_FILENO);
    int status;
    int f = fork();
    if (f == 0){
        dup2(arr[1], STDOUT_FILENO);
        run(second);
        //fprintf(stderr, "%s\n", second);
        exit(0);
    }
    else{
        wait(&status);
        if(WIFSIGNALED(status)) return 1;
        dup2(arr[0], STDIN_FILENO);
        close(arr[1]);
        run(a);
        dup2(in, STDIN_FILENO);
    }
  //free(line2);
    return 1;
}
