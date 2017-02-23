#include "../libs/common.h"

int nclients;

typrdef struct client_s {
	int	client_id;
	int	client_desc;
	char	client_ip[16];
	char	client_name[64];
} client_t
typedef *client_t client_l;

client_l client_list;
client_l last_client;

char* get_ip(struct sockaddr_in socket_addr);

int add_client_routine(int client_desc,
		       struct sockaddr_in client_addr);

int main(int argc,
	 char const *argv[])
{
	int 			server_desc , client_sock, client_addr_len, ret;
	struct sockaddr_in	server_addr , client_addr;
  				
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

	// Inizializzazione porta socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1) {
		fprintf(stderr, "Could not create socket");
    		exit(EXIT_FAILURE);
	}
  	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	if(bind(socket_desc,(struct sockaddr *)&server_addr , sizeof(server)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	listen(socket_desc , MAX_CONN_QUEUE);

	// Ciclo sentinella
	while(1) {
    		client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    		if (client_sock < 0) {
        		perror("Unable to connect to client");
        		return 1;
    		}
		fprintf(stderr, "Incoming connection");
		
		client_l thread_arg = malloc(sizeof(client_t));
		thread_arg-> client_desc = client_desc;
		thread_arg-> client_addr = client_addr;
		pthread_t* init_client_thread = malloc(pthread_t);
		ret = ptread_create(init_client_thread, NULL, init_client_routine, thread_arg);
		memset(client_addr, 0, sizeof(client_addr));
  	}
  	exit(EXIT_SUCCESS);
}

void *init_client_routine(void *arg)
{
	client_t client = (client_l) arg
	pthread_exit(NULL);
}

char* get_ip(struct sockaddr_in socket_addr) {
	char ip[16];
	fprintf(ip,
		"%d.%d.%d.%d\0",
		int(socket_addr.sin_addr.s_addr&0xFF),
		int((socket_addr.sin_addr.s_addr&0xFF00)>>8),
  		int((socket_addr.sin_addr.s_addr&0xFF0000)>>16),
  		int((socket_addr.sin_addr.s_addr&0xFF000000)>>24));
	return ip;
};
