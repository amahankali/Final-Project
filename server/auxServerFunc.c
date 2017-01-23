#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

///////////////////////////LOGIN-RELATED FUNCTIONS///////////////////////////

char* cypher(char* x){
  int i = 0;
  char *ans = calloc(strlen(x),1);
  while(*(x+i)){
    ans[i] = (*(x+i) * 3) % 128;
  }
  return ans;
}

int checkUsername(char* x){
  int i;
  for(i = 0; i < strlen(x); i++){
    if(*(x + i) == ':'){
      return 0;
    }
  }
  return 1;
}

//signs up username
//returns 1 if works
//returns 0 if not
int signUp(char* username, char* password) {
  umask(0000);
  char x[] = ":";
  char newLine[] = "\n";
  int f = open("users.txt", O_APPEND | O_RDWR, 0666);
  printf("\nRegistering User...\n\n");
  if(checkUsername(username) && checkUsername(password)){
    write(f, username, sizeof(username));
    write(f, x, sizeof(x));
    write(f, cypher(password), sizeof(password));
    write(f, newLine, sizeof(newLine));
    return 1;
  }
  return 0;
}

//checks if it is valid
//returns 1 if valid login
//returns 0 if not
int login(char* username, char* password) {
  umask(0000);
  char *a = calloc(10000, 1);
  int f = open("users.txt",  O_RDONLY, 0666);
  printf("\nLogging In User...\n\n");
  if(checkUsername(username) && checkUsername(password)){
    read(f, a, sizeof(a));
    while(*a){
      char* k = strsep(&a, "\n");
      if(strcmp(strsep(&k, ":"), username) == 0){
	if(strcmp(k, cypher(password)) == 0){
	  return 1;
	}
      }
    }
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
