# CLOUD9

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
