#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define OFFLINE	0
#define ONLINE 	1

typedef struct client_s {
  int     client_id;
  int     client_desc;
  int     client_status;
  char    client_port[5];
  char    client_name[11];
  struct  client_s* next;
  struct  client_s* prev;
} client_t;
typedef client_t* client_l;

int       nclients;
sem_t     client_list_semaphore;
client_l  client_list;
client_l  last_client;

int add_cl (client_l client);
int remove_cl (int id);
int send_cl (int sock_desc);

#endif
