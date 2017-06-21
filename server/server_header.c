#include "server_header.h"

// Routine principale del server
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

  int       ret;
  int       bytes_read;
  int       bytes_send;
  int       client_desc = client->client_desc;
  int*      client_id   = &client->client_id;
  char*     client_name = client->client_name;
  char      data[MAX_DATA_LEN];

  memset(data, 0, MAX_LEN_NAME);

  bytes_read = 0;

  // Recupero il nome da attribuire al client
  while (bytes_read < MAX_LEN_NAME) {
    ret = recv(client_desc, data + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) pthread_exit(NULL);
    if (ret == 0) pthread_exit(NULL);
    bytes_read++;
    if (data[bytes_read-1] == '\n' ||
	      data[bytes_read-1] == '\r' ||
	      data[bytes_read-1] == '\0')
      break;
  }
  data[bytes_read-1] = '\0';

  memcpy(client_name, data, bytes_read);

  bytes_send = 0;

  // Verifico se esiste gà un client con tale nome
  if (valid_name(client_name) <= 0) {
    int query_size = strlen(NAME_ALREADY_USED);
    char* query = malloc(sizeof(char)*query_size);
    fprintf(stderr, "Connection error: NAME_ALREADY_USED\n");
    memcpy(query, NAME_ALREADY_USED, query_size);
    while (bytes_send < query_size) {
      ret = send(client_desc, query + bytes_send, 1, 0);
      if (ret == -1 && errno == EINTR) continue;
      if (ret == -1) pthread_exit(NULL);
      bytes_send++;
    }
    pthread_exit(NULL);
  }

  add_cl(client);

  fprintf(stderr, "New client %s connected\n", client_name);

  while (1) {
    bytes_read = 0;
    memset(data, 0, MAX_DATA_LEN);
    while (1) {
      ret = recv(client_desc, data + bytes_read, 1, 0);
      if (ret == -1 && errno == EINTR) continue;
      if (ret == -1) pthread_exit(NULL);
      if (ret == 0) pthread_exit(NULL);
      bytes_read++;
      if (data[bytes_read-1] == '\n' ||
	        data[bytes_read-1] == '\r' ||
	        data[bytes_read-1] == '\0')
        break;
    }

    fprintf(stderr, "%s\n", data);

    data[bytes_read-1] = '\n';

    if (strcmp(data, QUIT) == 0) {
      fprintf(stderr, "Client %s disconnected\n", client_name);
      remove_cl(*client_id);
      pthread_exit(NULL);
    }

    if (strcmp(data, LIST) == 0) send_cl(client->client_desc);
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
