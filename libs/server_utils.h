#include "common.h"

typedef struct client_s {
	int	client_id;
	int	client_desc;
	char	client_ip[16];
	char	client_name[64];
	struct client_s* next;
	struct client_s* prev;
} client_t;
typedef client_t* client_l;

int nclients;
client_l client_list;
client_l last_client;
sem_t client_list_semaphore;

void remove_to_cl(int id);
void add_to_cl(client_l client);
void server_init(int* sock_desc, struct sockaddr_in* sock_addr);
void goodbye (void);
