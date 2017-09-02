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

#define MAX_CONN_QUEUE 10

/* Routine di inizializzazione del server */
int server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
  if (list_init()) return -1;
  *sock_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (*sock_desc == -1) exit(EXIT_FAILURE);
  sock_addr->sin_family = AF_INET;
  sock_addr->sin_addr.s_addr = INADDR_ANY;
  sock_addr->sin_port = htons(SERVER_PORT);
  if (bind(*sock_desc,
          (struct sockaddr *)sock_addr ,
          sizeof(*sock_addr)) < 0)
    return -1;
  if(listen(*sock_desc , MAX_CONN_QUEUE)) return -1;
  return 1;
}

/* Funzione di terminazione del server */
void server_exit () {
  free_list();
  fprintf(stderr, "Server Halted\n");
}

int server_routine(int argc, char const *argv[]) {

  int server_desc ,
      client_desc,
      client_addr_len,
      ret;

  struct sockaddr_in  server_addr ,
                      client_addr;

  // Configurazione del Server
  if (atexit(server_exit) != 0) exit(EXIT_FAILURE);
  if (signal(SIGINT, exit) == SIG_ERR) exit(EXIT_FAILURE);

  ret = server_init(&server_desc, &server_addr);
  if (ret < 1) exit(EXIT_FAILURE);
  fprintf(stderr, "Server Started\n");

  client_addr_len = sizeof(client_addr);

  // Metto il server in attesa di richieste di connessione
  while(1) {
    memset(&client_addr, 0, sizeof(client_addr));
    client_desc = accept(server_desc,
                         (struct sockaddr *)&client_addr,
                         (socklen_t*)&client_addr_len);
    if (client_desc < 0) continue;

    // Se arriva un client rimando la gestione all'apposito thread
    client_l thread_arg = malloc(sizeof(client_t));
    thread_arg->descriptor = client_desc;
    pthread_t* init_client_thread = malloc(sizeof(pthread_t));
    ret = pthread_create(init_client_thread,
                         NULL,
                         thread_routine,
                         (void*) thread_arg);
    if (ret != 0) continue;
    ret = pthread_detach(*init_client_thread);
  }
  exit(EXIT_SUCCESS);
}
