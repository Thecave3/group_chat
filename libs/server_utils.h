#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEBUG 					0
#define LOG 						0
#define SERVER_PORT     8583
#define MAX_CONN_QUEUE	10

#define OFFLINE	0
#define ONLINE 	1

typedef struct client_s {
	int			client_id;
	int			client_desc;
	int 		client_status;
	char		client_ip[17];
	char		client_name[63];
	struct 	client_s* next;
	struct 	client_s* prev;
} client_t;
typedef client_t* client_l;

int				nclients;
sem_t 		client_list_semaphore;
client_l 	client_list;
client_l 	last_client;

void 	client_list_wait();
void 	client_list_post();
int 	server_init(int* sock_desc, struct sockaddr_in* sock_addr);
int 	add_cl(client_l client);
int 	remove_cl(int id);
int 	send_cl(int sock_desc);
char* get_time();
