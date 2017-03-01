#include "../libs/common.h"
#include "../libs/server_protocol.h"
#include "../libs/server_utils.h"

#define WELCOME_MSG 	"THIS IS THE WELCOME MESSAGE OF SERVER\0"

int send_list(int socket_desc);
void *init_client_routine(void* arg);

int main(int argc, char const *argv[]) {
	int									server_desc , client_desc, client_addr_len, ret;
	struct sockaddr_in	server_addr , client_addr;

	atexit(goodbye);
  signal(SIGINT, exit);

	nclients = 0;
	client_addr_len = sizeof(client_addr);

	if (sem_init(&client_list_semaphore, 0, 1)) {
		if (DEBUG) {
			fprintf(stderr, "client_list_semaphore: error in sem_init;\n");
			fprintf(stderr, "\tmain\n");
		}
		exit(EXIT_FAILURE);
	}

	// Inizializzazione porta socket
	server_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (server_desc == -1) {
		fprintf(stderr, "Could not create socket");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVER_PORT);
	if(bind(server_desc,(struct sockaddr *)&server_addr , sizeof(server_addr)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	listen(server_desc , MAX_CONN_QUEUE);
	fprintf(stderr, "Server started!\n");
	// Ciclo sentinella
	while(1) {
		client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) {
			perror("Unable to connect to client");
			return 1;
    }
		fprintf(stderr, "Incoming connection by ");
		client_l thread_arg = malloc(sizeof(client_t));
		thread_arg->client_desc = client_desc;
		thread_arg->next = NULL;
		sprintf(thread_arg->client_ip,
			"%d.%d.%d.%d",
			(int)(client_addr.sin_addr.s_addr&0xFF),
			(int)((client_addr.sin_addr.s_addr&0xFF00)>>8),
			(int)((client_addr.sin_addr.s_addr&0xFF0000)>>16),
			(int)((client_addr.sin_addr.s_addr&0xFF000000)>>24));
		fprintf(stderr, "%s\n", thread_arg->client_ip);
		pthread_t* init_client_thread = malloc(sizeof(pthread_t));
		ret = pthread_create(init_client_thread, NULL, init_client_routine,(void*) thread_arg);
		if (ret != 0) {
			fprintf(stderr, "ptrhead_create: %s\n", strerror(ret));
			fprintf(stderr, "\tmain\n");
			exit(EXIT_FAILURE);
		}
		ret = pthread_detach(*init_client_thread);
		if (ret != 0) {
			fprintf(stderr, "ptrhead_detach: %s\n", strerror(ret));
			fprintf(stderr, "\tmain\n");
			exit(EXIT_FAILURE);
		}
		memset(&client_addr, 0, sizeof(client_addr));
  }
  exit(EXIT_SUCCESS);
}

void *init_client_routine(void *arg) {
	int ret;
	client_l client = (client_l) arg;
	send_message(client->client_desc, WELCOME_MSG, sizeof(WELCOME_MSG));
	recv_message(client->client_desc, client->client_name, sizeof(client->client_name));
	// Aggiungo il client alla scl del client attivi
	if (sem_wait(&client_list_semaphore)) {
		if (DEBUG) {
			perror("client_list_semaphore: error in wait");
			fprintf(stderr, "\tinit_client_routine\n");
		}
		exit(EXIT_FAILURE);
	}
	client->client_id = nclients;
	if (nclients == 0) {
		client_list = client;
		last_client = client;
	}
	else {
		last_client->next = client;
		last_client = client;
	}
	nclients++;
	if (sem_post(&client_list_semaphore)) {
		if (DEBUG) {
			perror("client_list_semaphore: error in post");
			fprintf(stderr, "\tinit_client_routine\n");
		}
		exit(EXIT_FAILURE);
	}
	char buffer[128];
	while(1) {
		ret = recv_message(client->client_desc, buffer, sizeof(buffer));
		if (ret == 0) {
			fprintf(stderr, "Connection closed by %s:%s\n", client->client_name, client->client_ip);
			break;
		}
	}
	pthread_exit(NULL);
}
