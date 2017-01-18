#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

char* cypher(char* x);
int checkUsername(char* x);
int reg(char* username, char* password);
int login(char* username, char* password);
