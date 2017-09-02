#ifndef LIST_H
#define LIST_H

#define OFFLINE	0
#define ONLINE 	1
#define MAX_LEN_NAME    11

typedef struct client_s {
  int     id;
  int     descriptor;
  int     status;
  char    name[MAX_LEN_NAME];
  struct  client_s* next;
  struct  client_s* prev;
} client_t;
typedef client_t* client_l;

int   list_init();
int   add_cl (client_l client);
int   remove_cl (int id);
int   set_status(int id, int status);
int   find_id_by_name(char* name);
int   valid_name(char* name);
int   get_descriptor(int id);
int   get_name(int id, char* buffer);
int   get_status(int id);
int   get_list(char* buffer);
void  free_list();

#endif
