#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../libs/server_utils.h"
#include "../libs/server_protocol.h"

#define MAX_CONN_QUEUE 10

void 	server_exit ();
void*	client_routine(void* arg);
int     server_init(int* sock_desc, struct sockaddr_in* sock_addr);
int 	server_routine(int argc, char const *argv[]);
#endif
