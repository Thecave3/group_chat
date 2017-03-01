#include "common.h"

typedef struct client_s {
	int	client_id;
	int	client_desc;
	char	client_ip[16];
	char	client_name[64];
	struct client_s* next;
} client_t;
typedef client_t* client_l;

int nclients;
client_l client_list;
client_l last_client;
sem_t client_list_semaphore;

void goodbye (void);
