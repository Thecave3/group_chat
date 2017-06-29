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
  int		quit_flag = 0;
  int       bytes_read;
  int       bytes_send;
  int       query_size;
  int       client_desc  = client->client_desc;
  int*      client_id    = &client->client_id;
  int*      partner_desc = &client->partner_desc;
  int*      start_connection;
  char*     client_name  = client->client_name;
  char      data[MAX_DATA_LEN];

  bytes_read = 0;
  memset(data, 0, MAX_LEN_NAME);

  // Recupero il nome da attribuire al client
  while (bytes_read < MAX_LEN_NAME) {
    ret = recv(client_desc, data + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      remove_cl(*client_id);
      pthread_exit(NULL);
    }
    if (ret == 0) {
      remove_cl(*client_id);
      pthread_exit(NULL);
    }
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
    query_size = strlen(NAME_ALREADY_USED);
    memset(data, 0, MAX_DATA_LEN);
    memcpy(data, NAME_ALREADY_USED, query_size);
    while (bytes_send < query_size) {
      ret = send(client_desc, data + bytes_send, 1, 0);
      if (ret == -1 && errno == EINTR) continue;
      if (ret == -1) {
        remove_cl(*client_id);
        pthread_exit(NULL);
      }
      bytes_send++;
    }
    pthread_exit(NULL);
  }
  *partner_desc = 0;
  add_cl(client);

  fprintf(stderr, "New client %s connected\n", client_name);

  while (1) {
	// Recupero i messaggi dal client
    bytes_read = 0;
	if (quit_flag == 0) {
      memset(data, 0, MAX_DATA_LEN);
      while (bytes_read < MAX_DATA_LEN) {
        ret = recv(client_desc, data + bytes_read, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        if (ret == 0) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_read++;
        if (data[bytes_read-1] == '\n' ||
	        data[bytes_read-1] == '\r' ||
	        data[bytes_read-1] == '\0')
          break;
      }
      data[bytes_read-1] = '\n';
    }
    quit_flag = 0;

    // QUIT: Chiudo la connessione con il client e lo rimuovo dalla client_list
    if (strncmp(data, QUIT, sizeof(QUIT)) == 0) {
      fprintf(stderr, "Client %s disconnected\n", client_name);
      remove_cl(*client_id);
      pthread_exit(NULL);
    }

    // LIST: Invio la client_list connessi al client;
    else if (strncmp(data, LIST, sizeof(LIST)) == 0) {
      fprintf(stderr, "Client %s requests client list\n", client_name);
      bytes_send = 0;
      query_size = strlen(LIST);
      while (bytes_send < query_size) {
        ret = send(client_desc, data + bytes_send, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_send++;
      }
      send_cl(client->client_desc);
      continue;
    }

    // YES: Ricezione ack "accetto richiesta di connessione"
    else if (strncmp(data, YES, sizeof(YES)) == 0 && *partner_desc != 0) {
	  start_connection = &client->start_connection;
      fprintf(stderr, "Client %s accept connection\n",client_name);
      set_status(*client_id, OFFLINE);
      bytes_send = 0;
      query_size = strlen(YES);
      while (bytes_send < query_size) {
        ret = send(*partner_desc, data + bytes_send, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_send++;
      }
      *start_connection = 2;
      while(1) {
		int message_size;
		bytes_read = 0;
		memset(data, 0, MAX_DATA_LEN);
		while (bytes_read < MAX_DATA_LEN) {
          ret = recv(client_desc, data + bytes_read, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          if (ret == 0) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_read++;
          if (data[bytes_read-1] == '\n') break;
        }
		message_size = strlen(data);
		fprintf(stderr,"%s: %s",client_name, data);
		if(*start_connection == 3) {
	      quit_flag = 1;
	      *partner_desc = 0;
	      set_status(*client_id, ONLINE);
	      break;
		}
		bytes_send = 0;
        while (bytes_send < message_size) {
          ret = send(*partner_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          } 
          bytes_send++;
        }
        if(strcmp(data,QUIT) == 0) {
	      *partner_desc = 0;
	      *start_connection = 3;
	      set_status(*client_id, ONLINE);
	      break;
		}
      }
    }

    // NO: Ricezione ack "rifiuto richiesta di connessione"
    else if (strncmp(data, NO, sizeof(NO)) == 0 && *partner_desc != 0) {
      start_connection = &client->start_connection;
      fprintf(stderr, "Client %s refuse connection\n",client_name);
      bytes_send = 0;
      query_size = strlen(NO);
      while (bytes_send < query_size) {
        ret = send(*partner_desc, data + bytes_send, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_send++;
      }
      *partner_desc = 0;
      *start_connection = 1;
    }
    // CONNECT nome: richiesta di connessione verso un altro utente
    else if (strncmp(CONNECT, data, (sizeof(CONNECT) - 1))== 0) {
      data[bytes_read - 1] = '\0';
      char* request_name = data + sizeof(CONNECT) - 1;
      client_l aux = find_cl_by_name(request_name);
      // Se il nome comunicato non è presente nella lista lo segnalo al client
      if (aux == NULL || aux->client_status == OFFLINE) {
        bytes_send = 0;
        query_size = strlen(CLIENT_NOT_EXIST);
        memset(data, 0, MAX_DATA_LEN);
        memcpy(data, CLIENT_NOT_EXIST, query_size);
        while (bytes_send < query_size) {
          ret = send(client_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
        }
        fprintf(stderr, "Client %s connection error: %s", client_name, data);
        continue;
      }
      // Se il client prova una connessione verso se stesso lo segnalo al client
      else if (aux->client_id == *client_id) {
        bytes_send = 0;
        query_size = strlen(CONNECT_WITH_YOURSELF);
        memset(data, 0, MAX_DATA_LEN);
        memcpy(data, CONNECT_WITH_YOURSELF, query_size);
        while (bytes_send < query_size) {
          ret = send(client_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
        }
        fprintf(stderr, "Client %s connection error: %s", client_name, data);
        continue;
      }
      // Altrimenti inoltro al client la richiesta di connessione
      else {
        aux->partner_desc = client_desc;
        set_status(*client_id, OFFLINE);
        set_status(aux->client_id, OFFLINE);
        *partner_desc = aux->client_desc;
        start_connection = &aux->start_connection;
        *start_connection = 0;
        bytes_send = 0;
        query_size = strlen(CONNECT);
        memset(data, 0, MAX_DATA_LEN);
        memcpy(data, CONNECT, query_size);
        strncat(data, client_name, MAX_LEN_NAME);
        strncat(data, "\n", 1);
        while (1) {
          ret = send(*partner_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
          if (data[bytes_send-1] == '\n') break;
        }
        fprintf(stderr, "Client %s request connection with %s\n", client_name, aux->client_name);
        while(1) {
		  if (*start_connection == 1) {
		    set_status(*client_id, ONLINE);
			set_status(aux->client_id, ONLINE);
			start_connection = &client->start_connection;
			fprintf(stderr,"Client %s refuse connection\n", aux->client_name);
			break;
		  }
		  if (*start_connection == 2) {
		    int message_size;
		    bytes_read = 0;
		    memset(data, 0, MAX_DATA_LEN);
		    while (bytes_read < MAX_DATA_LEN) {
              ret = recv(client_desc, data + bytes_read, 1, 0);
              if (ret == -1 && errno == EINTR) continue;
              if (ret == -1) {
                remove_cl(*client_id);
                pthread_exit(NULL);
              }
              if (ret == 0) {
                remove_cl(*client_id);
                pthread_exit(NULL);
              }
              bytes_read++;
              if (data[bytes_read-1] == '\n') break;
            }  
		    message_size = strlen(data);
		    fprintf(stderr,"%s: %s",client_name, data);
		    if(*start_connection == 3) {
	          *partner_desc = 0;
	          quit_flag = 1;
	          set_status(*client_id, ONLINE);
			  set_status(aux->client_id, ONLINE);
	          *start_connection = 3;
	          break;
		    }
		    bytes_send = 0;
            while (bytes_send < message_size) {
              ret = send(*partner_desc, data + bytes_send, 1, 0);
              if (ret == -1 && errno == EINTR) continue;
              if (ret == -1) {
                remove_cl(*client_id);
                pthread_exit(NULL);
              } 
              bytes_send++;
            }
            if(strcmp(data,QUIT) == 0){
			  *partner_desc = 0;
	          set_status(*client_id, ONLINE);
			  set_status(aux->client_id, ONLINE);
			  *start_connection = 3;
	          break;
            }
          }
	    }
      }
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
