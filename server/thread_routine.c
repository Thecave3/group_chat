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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "main_routine.h"
#include "thread_routine.h"
#include "../libs/list.h"
#include "../libs/protocol.h"

void *thread_routine(void* arg) {
  client_l  client = (client_l) arg;

  int   ret;
  int		quit_flag = 0;
  int   bytes_read;
  int   bytes_send;
  int   query_size;
  int   descriptor = client->descriptor;
  int   id = client->id;
  char  *name = client->name;

  recv_message(descriptor, name, MAX_LEN_NAME);

  pthread_exit(NULL);
}
