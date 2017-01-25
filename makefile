compile:
	gcc server/server.c server/constant.h server/auxServerFunc.c
	gcc client/client.c client/constant.h
