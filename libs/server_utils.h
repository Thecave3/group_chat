#include "common.h"

#define OFFLINE	0
#define ONLINE 	1

typedef struct client_s {
	int	client_id;
	int	client_desc;
	int client_status;
	char	client_ip[17];
	char	client_name[63];
	struct client_s* next;
	struct client_s* prev;
} client_t;
typedef client_t* client_l;

int nclients;
client_l client_list;
client_l last_client;
sem_t client_list_semaphore;

int send_cl(int sock_desc);
void remove_cl(int id);
void add_cl(client_l client);
void server_init(int* sock_desc, struct sockaddr_in* sock_addr);
void goodbye (void);
