#include "../libs/common.h"

#define WELCOME_MSG 	"THIS IS THE WELCOME MESSAGE OF SERVER\0"

int nclients;

typedef struct client_s {
	int	client_id;
	int	client_desc;
	char	client_ip[16];
	char	client_name[64];
	struct client_s* next;
} client_t;
typedef client_t* client_l;

client_l client_list;
client_l last_client;
sem_t client_list_semaphore;

char* get_ip(struct sockaddr_in socket_addr, char buffer);
int recv_message(int socket_desc, char* buffer, int buffer_len);
int send_message(int socket_desc, char* buffer, int buffer_len);

void *init_client_routine(void* arg);

void goodbye (void) {
	fprintf(stderr, "goodbye\n");
}

int main(int argc, char const *argv[]) {
	int									server_desc , client_desc, client_addr_len, ret;
	struct sockaddr_in	server_addr , client_addr;

	atexit(goodbye);

	nclients = 0;
	client_list = malloc(sizeof(client_t));
	client_addr_len = sizeof(client_addr);

	// Controllo sui valori in input
	if (argc != 2) {
		fprintf(stderr, "usage: central_server: PORT\n");
		exit(EXIT_FAILURE);
	}
	ret = atoi(argv[1]);
	if(ret > 1024 && ret < 65535) {
		fprintf(stderr, "Incorrect port value\n");
		exit(EXIT_FAILURE);
	}

	if (sem_init(&client_list_semaphore, 0, 1)) {
		if (DEBUG) {
			fprintf(stderr, "client_list_semaphore: error in sem_init;\n");
			fprintf(stderr, "\tmain\n", );
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
	server_addr.sin_port = htons(ret);
	if(bind(server_desc,(struct sockaddr *)&server_addr , sizeof(server_addr)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	listen(server_desc , MAX_CONN_QUEUE);

	// Ciclo sentinella
	while(1) {
		client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) {
			perror("Unable to connect to client");
			return 1;
    }
		fprintf(stderr, "Incoming connection");
		client_l thread_arg = malloc(sizeof(client_t));
		thread_arg->client_id = nclients;
		thread_arg->client_desc = client_desc;
		sprintf(thread_arg->client_ip,
			"%d.%d.%d.%d\0",
			(int)(client_addr.sin_addr.s_addr&0xFF),
			(int)((client_addr.sin_addr.s_addr&0xFF00)>>8),
			(int)((client_addr.sin_addr.s_addr&0xFF0000)>>16),
			(int)((client_addr.sin_addr.s_addr&0xFF000000)>>24));
		pthread_t* init_client_thread = malloc(sizeof(pthread_t));
		ret = pthread_create(init_client_thread, NULL, init_client_routine,(void*) thread_arg);
		if (ret != 0) {
			fprintf(stderr, "ptrhead_create: %s\n", msg, strerror(ret));
			fprintf(stderr, "\tmain\n");
			exit(EXIT_FAILURE);
		}
		ret = pthread_detach(*init_client_thread);
		if (ret != 0) {
			fprintf(stderr, "ptrhead_detach: %s\n", msg, strerror(ret));
			fprintf(stderr, "\tmain\n");
			exit(EXIT_FAILURE);      
		}
		memset(&client_addr, 0, sizeof(client_addr));
		nclients++;
  }
	ret = sem_destroy(&client_list_semaphore);
  exit(EXIT_SUCCESS);
}

void *init_client_routine(void *arg) {
	int ret;
	client_l client = (client_l) arg;
	if (sem_wait(&client_list_semaphore)) {
		if (DEBUG) {
			perror("client_list_semaphore: error in wait");
			fprintf(stderr, "\tinit_client_routine\n");
		}
		exit(EXIT_FAILURE);
	}
	if (nclients == 0) {
		client_list = client;
		last_client = client_list;
	}
	else {
		last_client->next = client;
		last_client = last_client->next;
	}
	if(sem_post(&client_list_semaphore)) {
		if (DEBUG) {
			perror("client_list_semaphore: error in post");
			fprintf(stderr, "\tinit_client_routine\n");
		}
		exit(EXIT_FAILURE);
	}
	if(send_message(client->client_desc, WELCOME_MSG, sizeof(WELCOME_MSG))){
		if (DEBUG)
			fprintf(stderr, "\tinit_client_routine\n");
		exit(EXIT_FAILURE);
	}
	if(recv_message(client->client_desc, client->client_name, sizeof(client->client_name))) {
		if (DEBUG)
			fprintf(stderr, "\tinit_client_routine\n");
		exit(EXIT_FAILURE);
	}
	pthread_exit(NULL);
}

int recv_message(int socket_desc, char* buffer,  int buffer_len) {
  int   ret;
  int   bytes_read = 0;

  while(1) {
    ret = recv(socket_desc, buffer + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret == -1) {
			if (DEBUG)
      	perror("recv_message: error in recv");
      return -1;
    }
    if (ret == 0) {
			if (DEBUG)
				perror("recv_message: connection closed by client");
      return 0;
    }
    bytes_read++;
    if (buffer[bytes_read-1] == '\n' ||
				buffer[bytes_read-1] ==  '\0' ||
				bytes_read == buffer_len)
			break;
  }
  buffer[bytes_read-1] = '\0';
  return bytes_read;
}

int send_message(int socket_desc, char* buffer, int buffer_len) {
  int   ret;
  int   bytes_send = 0;

  while (bytes_send < buffer_len) {
    ret = send(socket_desc, buffer + bytes_send, buffer_len - bytes_send, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
			if (DEBUG)
      	perror("send_message: error in send");
      return -1;
    }
    bytes_send += ret;
  }
  return bytes_send;
}
