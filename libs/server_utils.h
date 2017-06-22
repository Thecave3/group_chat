#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define OFFLINE	0
#define ONLINE 	1
#define MAX_DATA_LEN    256
#define MAX_LEN_NAME    10

typedef struct client_s {
  int     client_id;
  int     client_desc;
  int     client_status;
  char    client_name[MAX_LEN_NAME];
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
int valid_name(char* name);
int find_id_by_name(char* name);
client_l find_cl_by_name(char* name);
client_l find_cl(int cl);
int set_status(int id);

#endif
