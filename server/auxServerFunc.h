//ALL FILENAMES ARE FILE NAMES RELATIVE TO ROOT DIRECTORY IN OUR SERVER
//FOR ALL OF THE FUNCTIONS, RETURN 1 IF SUCCESS, 0 IF SOMETHING WRONG

//register user
int signUp(char* user, char* password);

//given an entered username and password, see if this is a valid combination
int login(char* user, char* password);

//make a file
int touch(char* filename);
