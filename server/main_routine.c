#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "main_routine.h"
#include "thread_routine.h"
#include "../libs/logger.h"
#include "../libs/list.h"
#include "../libs/protocol.h"

#define EXTINTED  2
#define ALIVE     1
#define ZOMBIE    0

typedef struct thread_s {
  pthread_t* thread_handler;
  client_l thread_arg;
  struct thread_s* next;
  struct thread_s* prev;
} thread_t;
typedef thread_t* thread_l;

thread_l  thread_list;

int server_desc ,
    client_desc,
    client_addr_len,
    ret;

struct sockaddr_in  server_addr ,
                    client_addr;

client_l thread_arg;
pthread_t* init_client_thread;
thread_l thread_list_element;

thread_l aux;
thread_l bux;
thread_l cux;

logger_t logger_server;

/* Funzione di terminazione del server */
void server_exit () {
  free_list();
  while(thread_list != NULL) {
    aux = thread_list;
    thread_list = thread_list->next;
    free(aux);
  }
  write_logger(logger_server,"Il server è stato terminato\n");
  close_logger(logger_server);
}

void garbage_collector (int ignored) {
  int ret;
  cux = thread_list;
  while(cux != NULL) {
    if (cux->thread_arg->alive == ALIVE) cux->thread_arg->alive = ZOMBIE;
    else {
      if (cux->thread_arg->alive == ZOMBIE) {
        ret = pthread_cancel(*(cux->thread_handler));
        if (ret != 0) exit(EXIT_FAILURE);
        write_logger(cux->thread_arg->log,"Il client è stato terminato per inattività\n");
        write_logger(logger_server,"Il Thread del client %s è stato ucciso per inattività\n", cux->thread_arg->name);
        close_logger(cux->thread_arg->log);
      }
      remove_cl(cux->thread_arg->id);
      ret = shutdown (cux->thread_arg->descriptor, 2);
      bux = cux;
      if (cux->next == NULL && cux->prev == NULL) thread_list = NULL;
			else if (cux->prev == NULL) {
       thread_list = cux->next;
			 thread_list->prev = NULL;
      }else if (cux->next == NULL) {
        cux = bux->prev;
        cux->next = NULL;
      }
      else {
        cux = bux->next;
        cux->prev = bux->prev;
        cux = bux->prev;
        cux->next = bux->next;
      }
      if(bux->thread_arg->alive == ORPHAN) write_logger(logger_server,"Le risorse del client John Doe sono state liberate\n", bux->thread_arg->name);
      else write_logger(logger_server,"Le risorse del client %s sono state liberate\n", bux->thread_arg->name);
      free(bux);
    }
    cux = cux->next;
  }
  alarm(20);
}

int main_routine(int argc, char const *argv[], int folder) {

  if (daemon(1,0)) exit(EXIT_FAILURE);

  logger_server = new_logger("Server_LOG", NULL);

  // Configurazione del Server
  if (atexit(server_exit) != 0) exit(EXIT_FAILURE);
  if (signal(SIGINT, exit) == SIG_ERR) exit(EXIT_FAILURE);
  if (signal(SIGALRM, garbage_collector) == SIG_ERR) exit(EXIT_FAILURE);
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) exit(EXIT_FAILURE);
  if (!list_init()) exit(EXIT_FAILURE);

  write_logger(logger_server,"Setup dei segnali completato\n");

  thread_list = NULL;

  alarm(20);

  write_logger(logger_server,"Garbage Collector avviato\n");

  server_init(&server_desc, &server_addr);

  write_logger(logger_server,"Inizializzazione connessioni completata\n");

  client_addr_len = sizeof(client_addr);

  // Metto il server in attesa di richieste di connessione
  while(1) {
    memset(&client_addr, 0, sizeof(client_addr));
    client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) continue;
    write_logger(logger_server,"Nuova connessione in ingresso\n");
    thread_arg = malloc(sizeof(client_t));
    init_client_thread = malloc(sizeof(pthread_t));
    thread_list_element = malloc(sizeof(thread_t));
    thread_arg->descriptor = client_desc;
    thread_list_element->thread_handler = init_client_thread;
    thread_list_element->thread_arg = thread_arg;
    thread_list_element->next = NULL;
    thread_list_element->prev = NULL;
    if (thread_list == NULL) thread_list = thread_list_element;
    else {
      aux = thread_list;
      while (aux->next != NULL) aux = aux->next;
		  aux->next = thread_list_element;
      thread_list_element->prev = aux;
    }
    ret = pthread_create(init_client_thread, NULL, thread_routine, (void*) thread_arg);
    if (ret != 0) continue;
    write_logger(logger_server,"Nuovo Thread avviato\n");
    ret = pthread_detach(*init_client_thread);
    if (ret != 0) continue;
  }
  exit(EXIT_SUCCESS);
}
