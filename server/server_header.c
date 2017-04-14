#include "server_header.h"

/* Routine principale del server */
int 	server_routine(int argc, char const *argv[]) {
  int                 server_desc ,                                              // Descrittore del server
                      client_desc,                                               // Descrittore del client
                      client_addr_len,                                           // Lunghezza della struttura sockaddr_in del client
                      ret;                                                       // Lunghezza della struttura
	struct sockaddr_in	server_addr ,                                              // Struttura sockaddr_in del server
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
		sprintf(thread_arg->client_ip,                                               // Mi ricavo l'ip del client
			"%d.%d.%d.%d",
			(int)(client_addr.sin_addr.s_addr&0xFF),
			(int)((client_addr.sin_addr.s_addr&0xFF00)>>8),
			(int)((client_addr.sin_addr.s_addr&0xFF0000)>>16),
			(int)((client_addr.sin_addr.s_addr&0xFF000000)>>24));
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
	}
	pthread_exit(NULL);
}

int		server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
	nclients = 0;
	client_list = NULL;
	last_client = NULL;

	if (sem_init(&client_list_semaphore, 0, 1)) return 0;

	// Inizializzazione porta socket
	*sock_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (*sock_desc == -1) exit(EXIT_FAILURE);
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_addr.s_addr = INADDR_ANY;
	sock_addr->sin_port = htons(SERVER_PORT);
	if(bind(*sock_desc,(struct sockaddr *)sock_addr , sizeof(*sock_addr)) < 0) return -1;
	if(listen(*sock_desc , MAX_CONN_QUEUE)) return -1;
	return 1;
}

void 	server_exit () {
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
