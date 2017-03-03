#include "server_utils.h"

int		add_cl(client_l client) {
	if (sem_wait(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in wait");
		fprintf(stderr, "Impossibile registrare il client");
		return 0;
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
		return 0;
	}
	return 1;
}

int 	remove_cl(int id) {
	if (sem_wait(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in wait");
		fprintf(stderr, "Impossibile registrare il client");
		return 0;
	}
	client_l aux;

	if (client_list == NULL && last_client == NULL) return -1;
	aux = last_client;
	if (aux->client_id == id) {
		if (aux->prev != NULL)
			last_client = aux->prev;
		else {
			client_list = NULL;
			last_client = NULL;
		}
		free(aux);
		if (sem_post(&client_list_semaphore)) {
			if (DEBUG) perror("client_list_semaphore: error in post");
			fprintf(stderr, "Impossibile registrare il client");
			return 0;
		}
		nclients--;
		return 0;
	}
	aux = client_list;
	while (aux != NULL) {
		if (aux->client_id == id) {
				client_l bux = aux;
			  aux = bux->prev;
				aux->next = bux->next;
				aux = bux->next;
				aux->prev = bux->prev;
				free(bux);
				break;
		}
		aux = aux->next;
	}
	nclients--;
	if (sem_post(&client_list_semaphore)) {
		if (DEBUG) perror("client_list_semaphore: error in post");
		fprintf(stderr, "Impossibile registrare il client");
		return 0;
	}
	return 1;
}

int		server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
	nclients = 0;
	client_list = NULL;
	last_client = NULL;

	if (sem_init(&client_list_semaphore, 0, 1)) {
		if (DEBUG) {
			fprintf(stderr, "client_list_semaphore: error in sem_init;\n");
			fprintf(stderr, "\tmain\n");
		}
		return 0;
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
		return -1;
	}
	if(listen(*sock_desc , MAX_CONN_QUEUE)) {
		if (DEBUG) perror("sock_desc: error in listen");
		fprintf(stderr, "Impossibile avviare la connessione\n");
		return -1;
	}
	return 1;
}

int 	send_cl(int sock_desc) {
	int   		ret;
	int				list_len = 0;
	int   		bytes_send = 0;
	int				buffer_len = 0;
	char 			buffer[84];
	client_l	aux = client_list;

	while (aux != NULL) {
		if (aux->client_status == ONLINE) {
			memset(buffer,0,84);
			strcpy(buffer, aux->client_ip);
			strcat(buffer,"\n");
			strcat(buffer, aux->client_name);
			strcat(buffer,"\n\r");
			buffer_len = strlen(buffer);
			while (bytes_send < buffer_len) {
				ret = send(sock_desc, buffer + bytes_send, buffer_len - bytes_send, 0);
				if (ret == -1 && errno == EINTR) continue;
				if (ret == -1) {
					if (DEBUG) perror("send_cl: error in send");
					return -1;
				}
				bytes_send += ret;
			}
			list_len += bytes_send;
			bytes_send = 0;
		}
		aux = aux->next;
	}
	ret = 0;
	while (ret <= 0) {
		ret = send(sock_desc, "\0", 1, 0);
		if (ret == -1 && errno == EINTR) continue;
		if (ret == -1) {
			if (DEBUG) perror("send_message: error in send");
			return -1;
		}
	}
	return list_len + 1;
}

void 	goodbye (void) {
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
