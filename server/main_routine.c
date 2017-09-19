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

/* Funzione di terminazione del server */
void server_exit () {
  free_list();
  while(thread_list != NULL) {
    aux = thread_list;
    thread_list = thread_list->next;
    free(aux);
  }
}

void garbage_collector (int ignored) {
  int ret;
  cux = thread_list;
  while(cux != NULL) {
    if (cux->thread_arg->alive == ALIVE) cux->thread_arg->alive = ZOMBIE;
    else {
      if (cux->thread_arg->alive == ZOMBIE) {
        fprintf(stderr, "thread %s deleated\n",  cux->thread_arg->name);
        ret = pthread_cancel(*(cux->thread_handler));
        if (ret != 0) exit(EXIT_SUCCESS);
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
      fprintf(stderr, "Client %s was kicked in the ass\n", bux->thread_arg->name);
      free(bux);
    }
    cux = cux->next;
  }
  alarm(20);
}

int main_routine(int argc, char const *argv[], int folder) {

  daemon(1,0);

  // Configurazione del Server
  if (atexit(server_exit) != 0) exit(EXIT_FAILURE);
  if (signal(SIGINT, exit) == SIG_ERR) exit(EXIT_FAILURE);
  if (signal(SIGALRM, garbage_collector) == SIG_ERR) exit(EXIT_FAILURE);
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) exit(EXIT_FAILURE);
  if (!list_init()) exit(EXIT_FAILURE);

  thread_list = NULL;

  alarm(20);

  server_init(&server_desc, &server_addr);

  client_addr_len = sizeof(client_addr);

  // Metto il server in attesa di richieste di connessione
  while(1) {
    memset(&client_addr, 0, sizeof(client_addr));
    client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) continue;
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
    ret = pthread_detach(*init_client_thread);
    if (ret != 0) continue;
  }
  exit(EXIT_SUCCESS);
}
