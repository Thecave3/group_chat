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
#include <sys/time.h>

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

thread_l thread_list;

/* Funzione di terminazione del server */
void server_exit () {
  free_list();
  thread_l aux;
  while(thread_list != NULL) {
    aux = thread_list;
    thread_list = thread_list->next;
    free(aux);
  }
  fprintf(stderr, "Server Halted\n");
}

void timeout(int ignored)
{
  int ret;
  thread_l aux = thread_list;
  while(aux != NULL) {
    if (aux->thread_arg->alive == ALIVE) aux->thread_arg->alive = ZOMBIE;
    else {
      if (aux->thread_arg->alive == ZOMBIE) {
        fprintf(stderr, "thread %s deleated\n",  aux->thread_arg->name);
        ret = pthread_cancel(*(aux->thread_handler));
        if (ret != 0) exit(EXIT_SUCCESS);
      }
      remove_cl(aux->thread_arg->id);
      ret = shutdown (aux->thread_arg->descriptor, 2);
      thread_l bux = aux;
      if (aux->next == NULL && aux->prev == NULL) thread_list = NULL;
			else if (aux->prev == NULL) {
       thread_list = aux->next;
			 thread_list->prev = NULL;
      }else if (aux->next == NULL) {
        aux = bux->prev;
        aux->next = NULL;
      }
      else {
        aux = bux->next;
        aux->prev = bux->prev;
        aux = bux->prev;
        aux->next = bux->next;
      }
      fprintf(stderr, "Client %s was kicked in the ass\n", bux->thread_arg->name);
      free(bux);
    }
    aux = aux->next;
  }
  alarm(20);
}

int main_routine(int argc, char const *argv[]) {

  thread_list = NULL;

  signal(SIGALRM, timeout);
  alarm(20);

  int server_desc ,
      client_desc,
      client_addr_len,
      ret;


  struct sockaddr_in  server_addr ,
                      client_addr;

  // Configurazione del Server
  if (atexit(server_exit) != 0) exit(EXIT_FAILURE);
  if (signal(SIGINT, exit) == SIG_ERR) exit(EXIT_FAILURE);
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) exit(EXIT_FAILURE);
  if (!list_init()) exit(EXIT_FAILURE);

  server_init(&server_desc, &server_addr);
  fprintf(stderr, "Server Started\n");

  client_addr_len = sizeof(client_addr);

  // Metto il server in attesa di richieste di connessione
  while(1) {
    memset(&client_addr, 0, sizeof(client_addr));
    client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) continue;
    client_l thread_arg = malloc(sizeof(client_t));
    pthread_t* init_client_thread = malloc(sizeof(pthread_t));
    thread_l thread_list_element = malloc(sizeof(thread_t));
    thread_arg->descriptor = client_desc;
    thread_list_element->thread_handler = init_client_thread;
    thread_list_element->thread_arg = thread_arg;
    thread_list_element->next = NULL;
    thread_list_element->prev = NULL;
    if (thread_list == NULL) thread_list = thread_list_element;
    else {
      thread_l aux = thread_list;
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
