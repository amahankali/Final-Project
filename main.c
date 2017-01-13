#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include "pipe.h"

int reply = 0;

//returns -1 if there is a '>' or a '<' right before a '|' (separated by spaces)
int noAdjSymbol(char* command);


/*=========================================================
main(): Starts by clearing the terminal and immediately 
redirects to route. From there, it prints out the current 
directory and asks for user input. It records this 
information using the fgets function then uses the strsep
function to form an array of semicolon divided sections of
the input. The program loops through the array executing
passing in each part of the input to the run function 
(located in pipe.h)
    -Parameters: none
    -Returns: an integer to terminate the function
==========================================================*/

int main () {
    system("clear");
    printf("Shell—Version 1.0\n");
    printf("An Arvind Mahankali-Brandon Huang-Will Ostlund Project\n");
    printf("We hope you enjoy! :)\n");
    chdir(getenv("HOME"));
	while(1) //inside of while loop reads one-line instruction, splits it by ';', and gives the pieces to singleCommand function
	{
        if(reply == -1){return 0;}
        char buff[PATH_MAX + 1];
        char *dir = getcwd(buff, PATH_MAX);
        printf("%s: ", dir);
        char *a = (char *) calloc(1, 256);
        fgets(a, 255, stdin);
        
        char* ans[MAXCOMMANDS];
        *(strchr(a, '\n')) = 0;
        int i = 0;

    int syntaxError = noAdjSymbol(a);
    if(syntaxError)
    {
      free(a);
      continue;
    }
		while(a)
		{
			char *s = strsep(&a, ";");
			ans[i] = s;
			i++;
		}
    free(a);

		int n = 0;
		for(; n < i; n++)
		{
            run(ans[n]);

		}


	}

	return 0;
}

/*=========================================================
  noAdjSymbol(char* command): The goal of this function is to
  catch syntax errors that arise from redirection and piping. If
  there is a "|" character at the beginning of the string provided
  in the command line, or if there is a ">" or "<" character directly
  before a "|" character in the provided string, this function will
  return -1. This is done by splitting the given command by spaces.
  main() then uses this function to determine if the given command
  string has a syntax error of this type.
      -Parameters: char* command
      -Returns: an integer: returns -1 if the given string contains
      a syntax error of the form above, and 0 otherwise.
==========================================================*/

int noAdjSymbol(char* command)
{
  int i = 0;
  char* ans[MAXCOMMANDS];

  char* commandCopy = (char *) calloc(1, strlen(command));
  strcpy(commandCopy, command);

  while(commandCopy)
  {
    ans[i] = strsep(&commandCopy, " ");
    i++;
  }

  if(strcmp(ans[0], "|") == 0)
  {
    fprintf(stderr, "-bash: syntax error near unexpected token `|'\n");
    return -1;
  }

  int j = 1;
  for(; j < i; j++)
  {
    if(strcmp(ans[j], "<") == 0 || strcmp(ans[j], ">") == 0)
    {
      if(j == i - 1)
      {
        fprintf(stderr, "-bash: syntax error near unexpected token `newline'\n");
        return -1;
      }
      if(strcmp(ans[j + 1], "|") == 0)
      {
        fprintf(stderr, "-bash: syntax error near unexpected token '|'\n");
        return -1;
      }
    }
  }
  return 0;
}
