#include "server_utils.h"

void add_to_cl(client_l client) {
	if (sem_wait(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in wait");
		fprintf(stderr, "Impossibile registrare il client");
		pthread_exit(NULL);
	}
	client->client_id = nclients;
	if (nclients == 0) {
		client_list = client;
		last_client = client;
	}
	else {
		client->prev = last_client;
		last_client->next = client;
		last_client = client;
	}
	nclients++;
	if (sem_post(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in post");
		fprintf(stderr, "Impossibile registrare il client");
		pthread_exit(NULL);
	}
}


void remove_to_cl(int id) {
	if (sem_wait(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in wait");
		fprintf(stderr, "Impossibile registrare il client");
		pthread_exit(NULL);
	}
	client_l aux;

	if (client_list == NULL && last_client == NULL) return;
	aux = last_client;
	if (aux->client_id == id) {
		if (aux->prev != NULL)
			last_client = aux->prev;
		else {
			client_list = NULL;
			last_client = NULL;
		}
		free(aux);
		return;
	}
	while (aux != NULL) {

	}

	if (sem_post(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in post");
		fprintf(stderr, "Impossibile registrare il client");
		pthread_exit(NULL);
	}
}

void server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
	nclients = 0;
	client_list = NULL;
	last_client = NULL;

	if (sem_init(&client_list_semaphore, 0, 1)) {
		if (DEBUG) {
			fprintf(stderr, "client_list_semaphore: error in sem_init;\n");
			fprintf(stderr, "\tmain\n");
		}
		exit(EXIT_FAILURE);
	}

	// Inizializzazione porta socket
	*sock_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (*sock_desc == -1) {
		if (DEBUG) perror("sock_desc: error in socket");
		fprintf(stderr, "Impossibile avviare la connessione\n");
		exit(EXIT_FAILURE);
	}
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_addr.s_addr = INADDR_ANY;
	sock_addr->sin_port = htons(SERVER_PORT);
	if(bind(*sock_desc,(struct sockaddr *)sock_addr , sizeof(*sock_addr)) < 0)
	{
		if (DEBUG) perror("sock_desc: error in bind");
		fprintf(stderr, "Impossibile avviare la connessione\n");
		exit(EXIT_FAILURE);
	}
	if(listen(*sock_desc , MAX_CONN_QUEUE)) {
		if (DEBUG) perror("sock_desc: error in listen");
		fprintf(stderr, "Impossibile avviare la connessione\n");
		exit(EXIT_FAILURE);
	}
}

void goodbye (void) {
	fprintf(stderr, "\n");
	if (DEBUG) fprintf(stderr, "Svuoto la lista dei client\n");
	while (client_list != NULL) {
		client_l aux = client_list;
		if(DEBUG) fprintf(stderr, "%d %s %s\n",aux->client_id, aux->client_ip, aux->client_name);
		client_list = client_list->next;
		free(aux);
	}
	sem_destroy(&client_list_semaphore);
	fprintf(stderr, "The Server say goodbye!\n");
}
