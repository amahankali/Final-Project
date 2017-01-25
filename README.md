# OUR FILE-SHARING SERVICE

---

##by Team GOM (Neils Graham, Arvind Mahankali and Will Ostlund

#Repository Contents
- DESIGN
- README.md
- makefile
- **client**
  - client.c
  - constant.h
  - makefile
- **server**
  - auxServerFunc.c
  - auxServerFunc.h
  - constant.h
  - makefile
  - server.c
  - users.txt

##How to Use
**Compile**: Type make in the highest directory of the repository.

**Set up Connection**: In constant.h (in both the client and the server directories) change TESTIP to the desired IP address of the server

**Run Client**: Change to the "client" directory and type in the command "make run"

**Run Server**: Change to the "server" directory and type in the command "make run"

##User Interface
Initially, you will be given the options to log in or sign up.

To use our file-sharing service, enter these commands on the client side:
$gitProject -lgo: Logs out of user's account


$gitProject -crf FILENAME: creates file named FILENAME, belonging to user


$gitProject -edt FILENAME: allows user to edit the file named FILENAME in emacs. (User must be owner of file or have access to file.)


$gitProject -rmf FILENAME: removes file named FILENAME.


$gitProject -inv FILENAME OTHERUSER: gives OTHERUSER access to FILENAME, giving OTHERUSER the ability to edit. The user invoking this command must be the owner of FILENAME.

These events all occur at the location of the server. However, the commands are entered at the location of the client.

Multiple clients can connect to the server at once.

IMPORTANT: All file names must end in .txt.
