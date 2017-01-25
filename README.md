# CLOUD9

---

##by Team GOM (Neils Graham, Arvind Mahankali and Will Ostlund

#Repository Contents
- DESIGN
- constant.h
- README.md
- filesystem.txt
- **client**
  - a.out
  - client.c
  - constant.h
  - constant.h.gch
- **server**
  - a.out
  - auxServerFunc.c
  - auxServerFunc.h
  - constant.h
  - constant.h.gch
  - server.c
  - users.txt

##How to Use
**Compile**: Simply use the makefile by typing in "make"

**Set up Connection**: In constant.h (in both the client and the server directories) change TESTIP to the desired IP address of the server

**Run Client**: Type in the command "make client"

**Run Server**: On the machine with the same IP address as the one given to TESTIP type in "make server"
