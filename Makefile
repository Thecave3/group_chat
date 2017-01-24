CC = gcc -Wall -O0 -ggdb
LDFLAGS = -lpthread
CMN= common.h

all: clean server unix windows

windows:
	$(CC)  $(LDFLAGS) $(CMN) windows_client/main.c -o windows
unix:
	$(CC) $(LDFLAGS) $(CMN) unix_client/main.c -o unix
server:
	$(CC) $(LDFLAGS) $(CMN) central_server/main.c -o server


clean:
	rm -f windows unix server
