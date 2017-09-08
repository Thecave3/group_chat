#ifndef LIST_H
#define LIST_H
#include <semaphore.h>

#define LIST_LEN_NAME 11

typedef struct client_s {
  int     id;
  int     descriptor;
  int     status;
  sem_t   sem;
  char    name[LIST_LEN_NAME];
  struct  client_s* speaker;
  struct  client_s* next;
  struct  client_s* prev;
} client_t;
typedef client_t* client_l;

int       list_init();
int       add_cl (client_l client);
int       remove_cl (int id);
int       set_status(int id, int status);
client_l  find_cl_by_name(char *name);
client_l  find_cl_by_id(int id);
int       valid_name(char *name);
int       get_descriptor(int id);
int       get_name(int id, char *buffer);
int       get_status(int id);
int       get_list(char **buffer);
void      free_list();

#endif
