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
#include "../libs/logger.h"
#include "../libs/server_utils.h"
#include "../libs/server_protocol.h"

int 	server_routine(int argc, char const *argv[]);
void*	client_routine(void* arg);

#endif
