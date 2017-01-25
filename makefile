compile: server/server.c server/constant.h server/auxServerFunc.c client/client.c client/constant.h
	gcc server/server.c server/constant.h server/auxServerFunc.c
	gcc client/client.c client/constant.h

server: server/a.out
	./server/a.out
	
client: client/a.out
	./client/a.out
