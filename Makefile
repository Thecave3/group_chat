CC = gcc -Wall -O0 -ggdb
LDFLAGS = -lpthread
CMN= common.h

all: clean cartella server unix #windows

cartella:
	mkdir -p build

#windows:
#	$(CC)  $(LDFLAGS) $(CMN) windows_client.c -o build/windows
unix:
	$(CC) $(LDFLAGS) $(CMN) unix_client.c -o build/unix
server:
	$(CC) $(LDFLAGS) $(CMN) central_server.c -o build/server


clean:
	rm -rf build
