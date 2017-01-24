//this is the header file for our command line
//all file names are in reference to current working directory

//share a file
int share(char* filename, char* owner, char* collab, int permissions); //collab is the collaborator

//share a folder
int shareDir(char* foldername, char* owner, char* collab, int permissions);

//make a file
int touch(char* filename);

//make a folder
int mkdir(char* foldername);

//remove a file
int rm(char* filename);

//remove a folder
int rmdir(char* foldername);

//open a file
int edit(char* filename);

