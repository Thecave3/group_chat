#include "server_header.h"

/* Routine principale del server */
int server_routine(int argc, char const *argv[]) {

  int server_desc ,
      client_desc,
      client_addr_len,
      ret;

  struct sockaddr_in  server_addr ,
                      client_addr;

  client_list = NULL;
  last_client = NULL;

  if (atexit(server_exit) != 0) exit(EXIT_FAILURE);
  if (signal(SIGINT, exit) == SIG_ERR) exit(EXIT_FAILURE);

  ret = server_init(&server_desc, &server_addr);
  if (ret < 1) exit(EXIT_FAILURE);
  fprintf(stderr, "Server Started\n");
  daemon(0,1);

  client_addr_len = sizeof(client_addr);
  while(1) {
    memset(&client_addr, 0, sizeof(client_addr));
    client_desc = accept(server_desc,
                         (struct sockaddr *)&client_addr,
                         (socklen_t*)&client_addr_len);
    if (client_desc < 0) continue;
    client_l thread_arg = malloc(sizeof(client_t));
    thread_arg->client_desc = client_desc;
    pthread_t* init_client_thread = malloc(sizeof(pthread_t));
    ret = pthread_create(init_client_thread,
                         NULL,
                         client_routine,
                         (void*) thread_arg);
    if (ret != 0) continue;
    ret = pthread_detach(*init_client_thread);
  }
  exit(EXIT_SUCCESS);
}

/* Routine di gestione dei client */
void*	client_routine(void *arg) {
  client_l  client = (client_l) arg;

  int*      client_id = &client->client_id;
  int*      status = &client->client_status;
  int       client_desc = client->client_desc;
  int       bytes_read = 0;
  int       query_ret;
  int       query_send;
  int       query_recv;
  int       ret;
  char*     client_name = client->client_name;
  char      query[QUERY_LEN];
  char      data[PACKET_LEN];

  client->next = NULL;                                                     // Setto a NULL il campo next
  client->prev = NULL;

  // Recupero il nome da attribuire al client
  while (bytes_read < PACKET_LEN) {
    ret = recv(client_desc, data + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) pthread_exit(NULL);
    if (ret == 0) pthread_exit(NULL);
    bytes_read++;
  }

  // Verifico se esiste gà un client con tale nome
  memcpy(client_name, data, 11);
  if (invalid_name(client_name)) {
    memcpy("NMAU\0", query, 5);
      while (query_send < QUERY_LEN) {
      ret = send(client_desc, query + query_send, QUERY_LEN - query_send, 0);
      if (ret == -1 && errno == EINTR) continue;
      if (ret == -1) pthread_exit(NULL);
      query_send += ret;
    }
    free(client);
    pthread_exit(NULL);
  }
  // In caso contrario aggiungo il client al database
  add_cl(client);

  // Mi metto in attesa di eventuali messaggi da parte di quest'ultimo
  while (1) {
    query_recv = 0;
    memset(query, 0, QUERY_LEN);
    while (query_ret < QUERY_LEN) {
      query_ret = recv(client_desc, query + query_recv, 1, 0);
      if (query_ret == -1 && errno == EINTR) continue;
      if (query_ret == -1) pthread_exit(NULL);
      if (query_ret == 0) pthread_exit(NULL);
      query_recv++;
      if (query_recv == QUERY_LEN) break;
    }
    query[query_recv-1] = '\0';
    if (strcmp(query, "QUIT\0") == 0) {
      remove_cl(*client_id);
      break;
    }
    if (strcmp(query, "LIST\0") == 0) send_cl(client_desc);
    if (strcmp(query, "CONN\0") == 0) {
      int id_recv;
      int id_ret;
      char id[MAX_ID];
      while(1) {
        id_ret = recv(client_desc, id + id_recv, 1, 0);
        if (id_ret == -1 && errno == EINTR) continue;
        if (id_ret == -1) pthread_exit(NULL);
        if (id_ret == 0) pthread_exit(NULL);
        query_recv++;
        if (id[id_recv-1] == '\n' ||
            id[id_recv-1] == '\r' ||
            id[id_recv-1] == '\0' ||
				    id_ret == MAX_ID) break;
      }
      int client_id = atoi(id);
    }
  }
  pthread_exit(NULL);
}

/* Routine di inizializzazione del server */
int server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
  nclients = 0;

  if (sem_init(&client_list_semaphore, 0, 1)) return 0;

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
  sem_wait(&client_list_semaphore);
  client_l aux;
  while (client_list != NULL) {
    aux = client_list;
    client_list = client_list->next;
    free(aux);
  }
  sem_post(&client_list_semaphore);
  sem_destroy(&client_list_semaphore);
  fprintf(stderr, "Server Halted\n");
}
