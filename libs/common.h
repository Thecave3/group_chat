#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>




#define BUF_LEN 1024
#define ERR_MSG_LENGTH 1024
#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8583
#define MAX_CONN_QUEUE 10
#define MAX_LEN_LIST 256
#define MAX_LEN_NAME 8
