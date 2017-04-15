#include "server_header.h"

/* Routine principale del server */
int 	server_routine(int argc, char const *argv[]) {
  int                 server_desc ,                                              // Descrittore del server
                      client_desc,                                               // Descrittore del client
                      client_addr_len,                                           // Lunghezza della struttura sockaddr_in del client
                      ret;                                                       // Lunghezza della struttura
  struct sockaddr_in  server_addr ,                                              // Struttura sockaddr_in del server
                      client_addr;                                               // Struttura sockaddr_in del client

  client_list = NULL;                                                            // Setto client_list a NULL
  last_client = NULL;                                                            // Setto last_client a NULL

  if (atexit(server_exit) != 0) {                                                // Collego all'esecuzione della funzione di terminazione del programma la funzione server_exit
    exit(EXIT_FAILURE);                                                          // In caso di errore termino il programma
  }

  if (signal(SIGINT, exit) == SIG_ERR) {                                         // Collego al segnale SIGINT l'esecuzione della funzione di terminazione del programma
    exit(EXIT_FAILURE);
  }

  ret = server_init(&server_desc, &server_addr);                                 // Inizializzo il server tramite la funzione server_init()
  if (ret < 1) exit(EXIT_FAILURE);                                               // Se ret è minore di uno si è verificato un errore e termino il programma
  fprintf(stderr, "Server Started\n");                                           // Avverto l'utente che l'init del server si è concluso con successo
  daemon(0,1);                                                                   // Metto il server in modalità demone

  client_addr_len = sizeof(client_addr);                                         // Calcolo clinet_addr_len
  while(1) {
    memset(&client_addr, 0, sizeof(client_addr));                                // Resetto preventivamente la struttura client_addr                                                                     // Inizio del ciclo sentinella
    client_desc = accept(server_desc,                                            // Attendo un eventuale connessione in ingresso
                         (struct sockaddr *)&client_addr,
                         (socklen_t*)&client_addr_len);
    if (client_desc < 0) continue;                                               // In caso di errore salto all'iterazione successiva
    client_l thread_arg = malloc(sizeof(client_t));                              // Alloco la struttura per la memorizzazione delle informazioni riguardanti il client
    thread_arg->client_desc = client_desc;                                       // Inserisco all'interno di tale struttura il descrittore del client
    thread_arg->next = NULL;                                                     // Setto a NULL il campo next
    thread_arg->prev = NULL;                                                     // Setto a NULL il campo prev
    pthread_t* init_client_thread = malloc(sizeof(pthread_t));                   // Alloco la memoria necessaria per la creazione del thread di gestione del client
    ret = pthread_create(init_client_thread,                                     // Inizializzo il thread di gestione del client
                         NULL,
                         client_routine,
                         (void*) thread_arg);
    if (ret != 0) continue;                                                      // In caso di errore passo all'iterazione successiva
    ret = pthread_detach(*init_client_thread);                                   // Marchio il thread come detached
    if (ret != 0) continue;                                                      // In caso di errore passo all'iterazione successiva
  }
  exit(EXIT_SUCCESS);                                                            // Termino il server
}

/* Routine di gestione dei client */
void*	client_routine(void *arg) {
	client_l 	client = (client_l) arg;

	int*			status = &client->client_status;
	int 			ret;
	int 			client_desc = client->client_desc;
	int   		bytes_read = 0;
	int   		query_ret;
	int  		 	query_recv;
	int*			client_id = &client->client_id;
	char*			client_name = client->client_name;
	char*			client_port = client->client_port;
	char			query[5];
	char 			data[PACKET_LEN];

  while(bytes_read < PACKET_LEN) {
    ret = recv(client_desc, data + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) pthread_exit(NULL);
    if (ret == 0) pthread_exit(NULL);
		bytes_read++;
  }

	memcpy (client_port	,data	 , 4);
	memcpy (client_name	,data+4, 12);

	*status = ONLINE;

	add_cl(client);
	while (1) {
		query_recv = 0;
		memset(query, 0, QUERY_LEN);
	  while(1) {
	    query_ret = recv(client_desc, query + query_recv, 1, 0);
	    if (query_ret == -1 && errno == EINTR) continue;
	    if (query_ret == -1) pthread_exit(NULL);
	    if (query_ret == 0) pthread_exit(NULL);
	    query_recv++;
	    if (query[query_recv-1] == '\n' ||
					query[query_recv-1] ==  '\0' ||
					query_recv == QUERY_LEN)
				break;
	  }
	  query[query_recv-1] = '\0';
		if (strcmp(query, "QUIT\0") == 0) {
			remove_cl(*client_id);
			break;
		}
		if (strcmp(query, "STOF\0") == 0 && *status != OFFLINE) *status = OFFLINE;
		if (strcmp(query, "STON\0") == 0 && *status != ONLINE) *status = ONLINE;
		if (strcmp(query, "LIST\0") == 0) send_cl(client_desc);
		if (strcmp(query, "CONN\0") == 0) {

    }
	}
	pthread_exit(NULL);
}

/* Routine di inizializzazione del server */
int    server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
  nclients = 0;                                                                   // Resetto nclients
  client_list = NULL;                                                             // Resetto client_list
  last_client = NULL;                                                             // Resetto last_client

  if (sem_init(&client_list_semaphore, 0, 1)) return 0;                           // Creo il semaforo per la gestione della lista dei client

  *sock_desc = socket(AF_INET , SOCK_STREAM , 0);                                 // Inizializzo la porta socket per la connessione
  if (*sock_desc == -1) exit(EXIT_FAILURE);                                       // In caso di errore termino il server
  sock_addr->sin_family = AF_INET;                                                // Popolo la struttura sock_addr
  sock_addr->sin_addr.s_addr = INADDR_ANY;
  sock_addr->sin_port = htons(SERVER_PORT);
  if(bind(*sock_desc,                                                             // Effettuo la bind della socket e ritorno -1 in caso di errore
          (struct sockaddr *)sock_addr ,
          sizeof(*sock_addr)) < 0)
    return -1;
  if(listen(*sock_desc , MAX_CONN_QUEUE)) return -1;                              // Pongo la socket in modalità listen e ritorno -1 in caso di errore
  return 1;                                                                       // Se tutto è andato a buon fine ritorno 1
}

/* Funzione di terminazione del server */
void    server_exit () {
  sem_wait(&client_list_semaphore);                                              // Blocca il semaforo per la gestione della lista dei client
  client_l aux;                                                                  // Dichiaro una variabile di tipo client_l
  while (client_list != NULL) {                                                  // Fintanto che client_list è diverso da NULL quindi se c'é almeno un client connesso
    aux = client_list;                                                           // Faccio puntare aux alla stessa zona di memoria di client_list
    client_list = client_list->next;                                             // Sposto client_list sull'elemto successivo
    free(aux);                                                                   // Libero la zona di memoria puntata da aux
  }
  sem_post(&client_list_semaphore);                                              // Sblocco il semaforo per la gestione della lista dei client
  sem_destroy(&client_list_semaphore);                                           // Distruggo il semaforo per la gestione della lista dei client
  fprintf(stderr, "Server Halted\n");                                            // Stampo un messaggio per avvertire l'utente che il server è terminato
}
