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

#define DEBUG 1

void *thread_routine(void* arg) {
  client_l  client = (client_l) arg;

  int   ret;
  int   query_size;
  int   descriptor = client->descriptor;
  int   id = client->id;
  char  *name = client->name;
  char  data[BUFFER_LEN];
  char *data_pointer = NULL;


  memset(data, 0, BUFFER_LEN);

  ret = recv_message(descriptor, data, MAX_LEN_NAME);
  if (ret == -1) pthread_exit(NULL);

  // Verifico se esiste gà un client con tale nome
  if (valid_name(name)) {
    query_size = strlen(NAME_ALREADY_USED);
    memset(data, 0, BUFFER_LEN);
    memcpy(data, NAME_ALREADY_USED, query_size);
    ret = send_message(descriptor, data, query_size);
    if(ret == -1) {
      free(client);
      pthread_exit(NULL);
    }
  }

  ret = add_cl(client);
  if (ret == -1) {
    free(client);
    pthread_exit(NULL);
  }

  if (DEBUG) fprintf(stderr, "New client %s connected\n", name);

  while(1) {
    ret = recv_message(descriptor,data,BUFFER_LEN);
    if (ret == -1) {
      remove_cl(id);
      free(client);
      pthread_exit(NULL);
    }
    if (strncmp(data, QUIT, sizeof(QUIT)) == 0) {
      if (DEBUG) fprintf(stderr, "Client %s disconnected:", name);
      remove_cl(id);
      free(client);
      pthread_exit(NULL);
      if (DEBUG) fprintf(stderr, " OK\n");
    }

    else if (strncmp(data, LIST, sizeof(LIST)) == 0) {
      if (DEBUG) fprintf(stderr, "Client %s requests client list:", name);
      ret = get_list(data_pointer);
      ret = send_message(descriptor,data_pointer,ret);
      if (ret == -1) {
        remove_cl(id);
        free(client);
        pthread_exit(NULL);
      }
      if (DEBUG) fprintf(stderr, " OK\n");
    }

  }

  pthread_exit(NULL);
}
