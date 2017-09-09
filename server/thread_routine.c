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
#define BUFFER_LEN 1024

void *thread_routine(void* arg) {
  client_l  client = (client_l) arg;
  client_l  *speaker = &client->speaker;
  int       ret;
  int       query_size;
  int       *id = &client->id;
  int       *status = &client->status;
  int       descriptor = client->descriptor;
  char      data[BUFFER_LEN];
  char      *request_name;
  char      *name = client->name;
  char      *data_pointer;
  sem_t     *sem = &client->sem;

  memset(data, 0, BUFFER_LEN);

  ret = recv_message(descriptor, data, LIST_LEN_NAME, Z_FLAG | N_FLAG);
  if (ret == -1) pthread_exit(NULL);

  // Verifico se esiste gà un client con tale nome
  if (valid_name(data) == -1) {
    query_size = strlen(NAME_ALREADY_USED);
    memset(data, 0, BUFFER_LEN);
    memcpy(data, NAME_ALREADY_USED, query_size);
    ret = send_message(descriptor, data, query_size, Z_FLAG | N_FLAG);
    pthread_exit(NULL);
  }

  memset(name, 0, LIST_LEN_NAME);
  memcpy(name, data, LIST_LEN_NAME);

  ret = add_cl(client);
  if (ret == -1) pthread_exit(NULL);

  if (DEBUG) fprintf(stderr, "New client %s connected\n", name);

  while(1) {

    memset(data, 0, BUFFER_LEN);

    ret = recv_message(descriptor, data, BUFFER_LEN, Z_FLAG | N_FLAG);
    if (ret == -1) {
      remove_cl(*id);
      pthread_exit(NULL);
    }

    data[ret-1] = '\n';

    if (strncmp(data, QUIT, sizeof(QUIT)) == 0 && *status == BUSY) {
      if (sem_wait(&(*speaker)->sem) == -1) {
        remove_cl((*speaker)->id);
        pthread_exit(NULL);
      }
      (*speaker)->speaker = NULL;
      (*speaker)->status = ONLINE;
      if (sem_post(&(*speaker)->sem) == -1) {
        remove_cl((*speaker)->id);
        pthread_exit(NULL);
      }
      ret = send_message((*speaker)->descriptor, data, BUFFER_LEN, Z_FLAG | N_FLAG);
      if (ret == -1) {
        remove_cl((*speaker)->id);
        pthread_exit(NULL);
      }
      if (sem_wait(sem) == -1) {
        remove_cl(*id);
        pthread_exit(NULL);
      }
      *speaker = NULL;
      *status = ONLINE;
      if (sem_post(sem) == -1) {
        remove_cl(*id);
        pthread_exit(NULL);
      }
    }

    else if (*status == BUSY && strncmp(data, NO, sizeof(NO)) == 0) {
      if (sem_wait(&(*speaker)->sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        (*speaker)->speaker = NULL;
        (*speaker)->status = ONLINE;
        if (sem_post(&(*speaker)->sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        ret = send_message((*speaker)->descriptor, data, BUFFER_LEN, Z_FLAG | N_FLAG);
        if (ret == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        if (sem_wait(sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        *speaker = NULL;
        *status = ONLINE;
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
    }

    else if (*status == BUSY) {
      ret = send_message((*speaker)->descriptor, data, BUFFER_LEN, Z_FLAG | N_FLAG);
      if (ret == -1) {
        remove_cl((*speaker)->id);
      }
    }

    else if (*status == ONLINE && strncmp(data, QUIT, sizeof(QUIT)) == 0) {
      if (DEBUG) fprintf(stderr, "Client %s disconnected:", name);
      remove_cl(*id);
      if (DEBUG) fprintf(stderr, " OK\n");
      pthread_exit(NULL);
    }

    else if (*status == ONLINE && strncmp(data, LIST, sizeof(LIST)) == 0) {
      if (DEBUG) fprintf(stderr, "Client %s requests client list:", name);
      ret = get_list(&data_pointer);
      ret = send_message(descriptor, data_pointer, ret, Z_FLAG);
      if (ret == -1) {
        remove_cl(*id);
        pthread_exit(NULL);
      }
      if (DEBUG) {
        fprintf(stderr, " OK\n");
        fprintf(stderr, "%s", data_pointer);
      }
    }

    else if (*status == ONLINE && strncmp(CONNECT, data, (sizeof(CONNECT) - 1))== 0) {

      data[ret - 1] = '\0';
      request_name = data + sizeof(CONNECT) - 1;
      if (sem_wait(sem) == -1) {
        remove_cl(*id);
        pthread_exit(NULL);
      }
      *speaker = find_cl_by_name(request_name);

      if (DEBUG) fprintf(stderr, "Try connection with %s:", (*speaker)->name);

      // Tentativo di connessione ad un client non presente nella Client List
      if (*speaker == NULL) {
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        if (DEBUG) fprintf(stderr, "%s", CLIENT_NOT_EXIST);
        query_size = strlen(CLIENT_NOT_EXIST);
        memset(data, 0, BUFFER_LEN);
        memcpy(data, CLIENT_NOT_EXIST, query_size);
        ret = send_message(descriptor, data, query_size, Z_FLAG | N_FLAG);
        if (ret == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
      }

      // Tentativo di connessione con se stessi
      else if ((*speaker)->id == *id) {
        *speaker = NULL;
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        if (DEBUG) fprintf(stderr, "%s", CONNECT_WITH_YOURSELF);
        query_size = strlen(CONNECT_WITH_YOURSELF);
        memset(data, 0, BUFFER_LEN);
        memcpy(data, CONNECT_WITH_YOURSELF, query_size);
        ret = send_message(descriptor,data ,query_size, Z_FLAG | N_FLAG);
        if (ret == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
      }

      // Tentativo di connessione ad un altro client presente nella Client List
      else {
        *status = BUSY;
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          pthread_exit(NULL);
        }
        if (sem_wait(&(*speaker)->sem) == -1) {
          remove_cl((*speaker)->id);
          pthread_exit(NULL);
        }
        if ((*speaker)->status == ONLINE) {
          (*speaker)->speaker = client;
          (*speaker)->status = BUSY;
          if (sem_post(&(*speaker)->sem) == -1) {
            remove_cl((*speaker)->id);
            pthread_exit(NULL);
          }
          data[ret - 1] = '\n';
          ret = send_message((*speaker)->descriptor, data, ret, Z_FLAG | N_FLAG);
          fprintf(stderr, "%d %d %s", (*speaker)->descriptor, ret, data);
          if (ret == -1) {
            remove_cl(*id);
            pthread_exit(NULL);
          }
        }
        else {
          if (sem_post(&(*speaker)->sem) == -1) {
            remove_cl((*speaker)->id);
            pthread_exit(NULL);
          }
          if (sem_wait(sem) == -1) {
            remove_cl(*id);
            pthread_exit(NULL);
          }
          *speaker = NULL;
          *status = BUSY;
          if (sem_post(sem) == -1) {
            remove_cl(*id);
            pthread_exit(NULL);
          }
          if (DEBUG) fprintf(stderr, "%s", CLIENT_BUSY);
          query_size = strlen(CLIENT_BUSY);
          memset(data, 0, BUFFER_LEN);
          memcpy(data, CLIENT_BUSY, query_size);
          ret = send_message(descriptor,data, query_size, Z_FLAG | N_FLAG);
          if (ret == -1) {
            remove_cl(*id);
            pthread_exit(NULL);
          }
        }
      }
    }
    fprintf(stderr, "%s:\n%d\n%d\n%d\n", client->name,
            client->id,
            client->status,
            client->descriptor);
  }
  pthread_exit(NULL);
}
