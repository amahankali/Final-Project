//ALL FILENAMES ARE FILE NAMES RELATIVE TO ROOT DIRECTORY IN OUR SERVER
//FOR ALL OF THE FUNCTIONS, RETURN 1 IF SUCCESS, 0 IF SOMETHING WRONG

//register user
int signUp(char* user, char* password);

//given an entered username and password, see if this is a valid combination
int login(char* user, char* password);

//share a file
int share(char* filename, char* owner, char* collab, int permissions); //collab is the collaborator

//share a folder
int shareDir(char* foldername, char* owner, char* collab, int permissions);

//share current working directory
int share(char* owner, char* collab, int permissions);

//check if anyone is editing this file
int checkFile(char* filename);

//list of files and directories shared with the user
char* sharedItems(char* user);

//make a file
int touch(char* filename);
