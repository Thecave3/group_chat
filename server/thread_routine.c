#include <stdio.h>
#include <fcntl.h>
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
#include "../libs/logger.h"
#include "../libs/protocol.h"

#define DEBUG 1
#define BUFFER_LEN 1024
#define PATH "log"

void *thread_routine(void* arg) {

  client_l  client = (client_l) arg;
  client_l  *speaker = &client->speaker;
  logger_t  log;
  int       ret;
  int       query_size;
  int       first_time = 0;
  int       descriptor = client->descriptor;
  int       *id = &client->id;
  int       *status = &client->status;
  int       *alive = &client->alive;
  char      data[BUFFER_LEN];
  char      *request_name;
  char      *name = client->name;
  char      *data_pointer;
  sem_t     *sem = &client->sem;

  memset(data, 0, BUFFER_LEN);

  // Ricevo il nome dal client
  ret = recv_message(descriptor, data, BUFFER_LEN, N_FLAG);
  if (ret == -1) pthread_exit(NULL);

  data[ret-1] = '\0';

  // Verifico se esiste gà un client con tale nome
  if (valid_name(data) == -1) {
    query_size = strlen(NAME_ALREADY_USED);
    memset(data, 0, BUFFER_LEN);
    memcpy(data, NAME_ALREADY_USED, query_size);
    ret = send_message(descriptor, data, query_size, N_FLAG);
    *alive = ORPHAN;
    pthread_exit(NULL);
  }

  memset(name, 0, LIST_LEN_NAME);
  memcpy(name, data, LIST_LEN_NAME);

  // Aggiungo il client alla lista dei client connessi
  ret = add_cl(client);
  if (ret == -1) pthread_exit(NULL);

  client->log = new_logger(name, "log");
  log = client->log;

  write_logger(log, "Il client si è connesso\n");

  // Inizio del ciclo sentinella
  while(1) {

    memset(data, 0, BUFFER_LEN);
    ret = recv_message(descriptor, data, BUFFER_LEN, N_FLAG);
    if (ret == -1) {
      remove_cl(*id);
      write_logger(log, "Il client si è disconnesso in modo anomalo\n");
      close_logger(log);
      pthread_exit(NULL);
    }
    *alive = ALIVE;

    if (strncmp(data, QUIT, strlen(QUIT)) == 0 && *status == BUSY) {
      write_logger((*speaker)->log, "Il client %s ha terminato la chat\n", name);
      write_logger(log, "Il client ha terminato la chat\n", name);
      if (sem_wait(&(*speaker)->sem) == -1) {
        remove_cl((*speaker)->id);
      }
      (*speaker)->speaker = NULL;
      (*speaker)->status = ONLINE;
      if (sem_post(&(*speaker)->sem) == -1) {
        remove_cl((*speaker)->id);
      }
      ret = send_message((*speaker)->descriptor, data, BUFFER_LEN, N_FLAG);
      if (ret == -1) {
        remove_cl((*speaker)->id);
      }
      if (sem_wait(sem) == -1) {
        remove_cl(*id);
        write_logger(log, "Errore critito: impossibile effettuare la sem_wait\n");
        close_logger(log);
        pthread_exit(NULL);
      }
      *speaker = NULL;
      *status = ONLINE;
      if (sem_post(sem) == -1) {
        remove_cl(*id);
        write_logger(log, "Errore critito: impossibile effettuare la sem_post\n");
        close_logger(log);
        pthread_exit(NULL);
      }
      first_time = 0;
    }

    else if (*status == BUSY && strncmp(data, NO, strlen(NO)) == 0) {
      write_logger((*speaker)->log, "Il client %s ha rifiutato la chat\n", name);
      write_logger(log, "Il client ha rifiutato la chat\n", name);
      if (sem_wait(&(*speaker)->sem) == -1) {
        remove_cl((*speaker)->id);
      }
      (*speaker)->speaker = NULL;
      (*speaker)->status = ONLINE;
      if (sem_post(&(*speaker)->sem) == -1) {
        remove_cl((*speaker)->id);
      }
      ret = send_message((*speaker)->descriptor, data, strlen(NO), N_FLAG);
      if (ret == -1) {
        remove_cl((*speaker)->id);
      }
      if (sem_wait(sem) == -1) {
        remove_cl(*id);
        write_logger(log, "Errore critito: impossibile effettuare la sem_wait\n");
        close_logger(log);
        pthread_exit(NULL);
      }
      *speaker = NULL;
      *status = ONLINE;
      if (sem_post(sem) == -1) {
        remove_cl(*id);
        write_logger(log, "Errore critito: impossibile effettuare la sem_post\n");
        close_logger(log);
        pthread_exit(NULL);
      }
    }

    else if (*status == BUSY) {
      if (first_time == 0 && strncmp(data, YES, strlen(YES)) == 0) {
        write_logger(log, "Il client ha accettato la chat\n");
        write_logger((*speaker)->log, "Il client %s ha accettato la chat\n", name);
        first_time++;
      }
      ret = send_message((*speaker)->descriptor, data, BUFFER_LEN, N_FLAG);
      (*speaker)->alive = ALIVE;
      if (ret == -1) {
        remove_cl((*speaker)->id);
      }
    }

    else if (*status == ONLINE && strncmp(data, QUIT, strlen(QUIT)) == 0) {
      remove_cl(*id);
      write_logger(log, "Il client si è disconnesso\n");
      close_logger(log);
      pthread_exit(NULL);
    }

    else if (*status == ONLINE && strncmp(data, LIST, strlen(LIST)) == 0) {
      ret = get_list(&data_pointer);
      ret = send_message(descriptor, data_pointer, ret, Z_FLAG);
      write_logger(log, "Richiesta inoltro della lista dei client\n");
      if (ret == -1) {
        remove_cl(*id);
        write_logger(log, "Il client si è disconnesso in modo anomalo\n");
        close_logger(log);
        pthread_exit(NULL);
      }
    }

    else if (*status == ONLINE && strncmp(CONNECT, data, (strlen(CONNECT) - 1))== 0) {
      data[ret - 1] = '\0';
      request_name = data + sizeof(CONNECT) - 1;
      if (sem_wait(sem) == -1) {
        remove_cl(*id);
        write_logger(log, "Errore critito: impossibile effettuare la sem_wait\n");
        close_logger(log);
        pthread_exit(NULL);
      }
      *speaker = find_cl_by_name(request_name);
      data[ret - 1] = '\n';

      // Tentativo di connessione ad un client non presente nella Client List
      if (*speaker == NULL) {
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          write_logger(log, "Errore critito: impossibile effettuare la sem_post\n");
          close_logger(log);
          pthread_exit(NULL);
        }
        query_size = strlen(CLIENT_NOT_EXIST);
        memset(data, 0, BUFFER_LEN);
        memcpy(data, CLIENT_NOT_EXIST, query_size);
        ret = send_message(descriptor, data, query_size, N_FLAG);
        if (ret == -1) {
          remove_cl(*id);
          write_logger(log, "Il client si è disconnesso in modo anomalo\n");
          close_logger(log);
          pthread_exit(NULL);
        }
        write_logger(log, "Richiesta di connessione verso un client inesistente\n");
      }

      // Tentativo di connessione con se stessi
      else if ((*speaker)->id == *id) {
        *speaker = NULL;
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          write_logger(log, "Errore critito: impossibile effettuare la sem_post\n");
          close_logger(log);
          pthread_exit(NULL);
        }
        query_size = strlen(CONNECT_WITH_YOURSELF);
        memset(data, 0, BUFFER_LEN);
        memcpy(data, CONNECT_WITH_YOURSELF, query_size);
        ret = send_message(descriptor,data ,query_size, N_FLAG);
        if (ret == -1) {
          remove_cl(*id);
          write_logger(log, "Il client si è disconnesso in modo anomalo\n");
          close_logger(log);
          pthread_exit(NULL);
        }
        write_logger(log, "Tentativo di connessione con se stessi\n");
      }

      // Tentativo di connessione ad un altro client presente nella Client List
      else {
        *status = BUSY;
        if (sem_post(sem) == -1) {
          remove_cl(*id);
          write_logger(log, "Errore critito: impossibile effettuare la sem_post\n");
          close_logger(log);
          pthread_exit(NULL);
        }
        if (sem_wait(&(*speaker)->sem) == -1) {
          remove_cl((*speaker)->id);
        }
        if ((*speaker)->status == ONLINE) {
          (*speaker)->speaker = client;
          (*speaker)->status = BUSY;
          if (sem_post(&(*speaker)->sem) == -1) {
            remove_cl((*speaker)->id);
          }
          data[ret - 1] = '\n';
          ret = send_message((*speaker)->descriptor, data, ret, N_FLAG);
          if (ret == -1) {
            remove_cl((*speaker)->id);
            write_logger(log, "Il client si è disconnesso in modo anomalo\n");
            close_logger(log);
            pthread_exit(NULL);
          }
          write_logger(log, "Tentativo di connessione con %s\n", (*speaker)->name);
          write_logger((*speaker)->log, "Tentativo di connessione da parte di %s\n", name);
        }
        else {
          if (sem_post(&(*speaker)->sem) == -1) {
            remove_cl((*speaker)->id);
          }
          if (sem_wait(sem) == -1) {
            remove_cl(*id);
            write_logger(log, "Errore critito: impossibile effettuare la sem_wait\n");
            close_logger(log);
            pthread_exit(NULL);
          }
          *speaker = NULL;
          *status = ONLINE;
          if (sem_post(sem) == -1) {
            remove_cl(*id);
            write_logger(log, "Errore critito: impossibile effettuare la sem_post\n");
            close_logger(log);
            pthread_exit(NULL);
          }
          query_size = strlen(CLIENT_BUSY);
          memset(data, 0, BUFFER_LEN);
          memcpy(data, CLIENT_BUSY, query_size);
          ret = send_message(descriptor,data, query_size, N_FLAG);
          if (ret == -1) {
            remove_cl(*id);
            write_logger(log, "Il client si è disconnesso in modo anomalo\n");
            close_logger(log);
            pthread_exit(NULL);
          }
          write_logger(log, "Tentativo di connessione con un client occupato\n");
        }
      }
    }
  }
  pthread_exit(NULL);
}
