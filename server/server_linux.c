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
	server_init(&server_desc, &server_addr);

	fprintf(stderr, "Server started!\n");

	// Ciclo sentinella
	client_addr_len = sizeof(client_addr);
	while(1) {
		client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) {
			if (DEBUG) perror("client_desc: error in accept");
			fprintf(stderr, "Impossibile connettersi al client");
			continue;
    }
		fprintf(stderr, "Incoming connection ");
		client_l thread_arg = malloc(sizeof(client_t));
		thread_arg->client_desc = client_desc;
		thread_arg->next = NULL;
		thread_arg->prev = NULL;
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
			if (DEBUG) fprintf(stderr, "init_client_thread: error in ptrhead_create: %s\n", strerror(ret));
			fprintf(stderr, "Impossibile connettersi al client");
			continue;
		}
		ret = pthread_detach(*init_client_thread);
		if (ret != 0) {
			if (DEBUG) fprintf(stderr, "init_client_thread: error in ptrhead_detach: %s\n", strerror(ret));
			fprintf(stderr, "Impossibile connettersi al client");
			continue;
		}
		memset(&client_addr, 0, sizeof(client_addr));
  }
  exit(EXIT_SUCCESS);
}

void *init_client_routine(void *arg) {
	int 			ret;
	char			query[5];
	client_l 	client = (client_l) arg;
	ret = recv_message(client->client_desc, client->client_name, sizeof(client->client_name));
	client->client_status = ONLINE;
	add_cl(client);
	while (1) {
		ret = recv_message(client->client_desc, query, 5);
		if (ret <= 0) pthread_exit (NULL);
		if (strcmp(query, "QUIT\0") == 0) {
			remove_cl(client->client_id);
			break;
		}
		if (strcmp(query, "STOF\0") == 0)
			client->client_status = OFFLINE;
		if (strcmp(query, "STON\0") == 0)
			client->client_status = ONLINE;
		if (strcmp(query, "LIST\0") == 0)
			send_cl(client->client_desc);
	}
	pthread_exit(NULL);
}
