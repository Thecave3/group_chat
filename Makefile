CC = gcc -Wall
OPT = -O0
LDFLAGS = -ggdb -lncurses -lpthread
S_COMMON = libs/server_protocol.c libs/server_utils.c
C_COMMON = libs/server_protocol.c
CMN = libs/common.h

all: clean cartella server_linux linux_client #windows

clean:
	rm -rf build
cartella:
	mkdir -p build
server_linux:
	$(CC) $(OPT) $(S_COMMON) server/server_linux.c -o build/server_linux $(LDFLAGS)
linux_client:
	$(CC) $(OPT) $(C_COMMON) client/linux_client.c -o build/linux_client $(LDFLAGS)
