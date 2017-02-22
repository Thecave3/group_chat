CC = gcc -Wall
OPT = -O0
LDFLAGS = -ggdb -lncurses -lpthread
CMN= libs/common.h

all: clean cartella server linux #windows

cartella:
	mkdir -p build

#windows:
#	$(CC)  $(LDFLAGS) $(CMN) windows_client.c -o build/windows
linux:
	$(CC) $(OPT) $(CMN) client/linux_client.c -o build/linux $(LDFLAGS)
server:
	$(CC) $(OPT) $(CMN) server/central_server.c -o build/server $(LDFLAGS)

clean:
	rm -rf build
