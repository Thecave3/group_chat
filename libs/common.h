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



#define CLIENT_PORT 8584
#define BUF_LEN 1024
#define ERR_MSG_LENGTH 1024
#define MAX_CONN_QUEUE 10
#define MAX_LEN_LIST 256
#define MAX_LEN_NAME 63
#define MAX_CLIENTS 5
