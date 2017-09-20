#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "list.h"
#include "server_common.h"

int       nclients;
int	      id_clients;
client_l  client_list;
client_l  last_client;
sem_t     client_list_semaphore;

int list_init() {
  nclients = 0;
  if (sem_init(&client_list_semaphore, 0, 1) == -1) return 0;
  return 1;
}

int add_cl(client_l client) {
  client->id = id_clients;
  client->status = ONLINE;
  client->alive = ALIVE;
  if (sem_init(&client->sem, 0, 1) == -1) return -1;
	if (sem_wait(&client_list_semaphore) == -1) return -1;
  if (nclients == 0) {
    client->next = NULL;
    client->prev = NULL;
    client_list = client;
    last_client = client;
  }
	else {
		client->prev = last_client;
    client->next = NULL;
    last_client->next = client;
    last_client = client;
  }
	nclients++;
  id_clients++;
  if (sem_post(&client_list_semaphore) == -1)return -1;
  return 1;
}

int remove_cl(int id) {
  int ret = 0;
  client_l aux, bux;
  aux = client_list;
	if (sem_destroy(&client_list_semaphore)) return -1;
	if (sem_wait(&client_list_semaphore) == -1) return -1;
  while (aux != NULL) {
    if (aux->id == id) {
      bux = aux;
      if (aux->next == NULL && aux->prev == NULL) {
        client_list = NULL;
        last_client = NULL;
      }
			else if (aux->prev == NULL) {
        client_list = aux->next;
        client_list->prev = NULL;
      }
			else if (aux->next == NULL) {
        last_client = aux->prev;
        last_client->next = NULL;
      }
			else {
        aux = bux->prev;
        aux->next = bux->next;
        aux = bux->next;
        aux->prev = bux->prev;
      }
      bux->alive = EXTINTED;
	    nclients--;
      ret++;
      break;
    }
		aux = aux->next;
    }
  if (sem_post(&client_list_semaphore) == -1) return -1;
  return ret;
}

int set_status(int id, int status) {
  client_l aux;
  int ret = 0;
	aux = client_list;
  if (status != 1 || status != 0) {
	  while (aux != NULL) {
		  if (aux->id == id) {
          if (sem_wait(&aux->sem) == -1) return -1;
	        aux->status = status;
          ret = 1;
          break;
          if (sem_post(&aux->sem) == -1) return -1;
      }
		  aux = aux->next;
	  }
  }
	return ret;
}

client_l find_cl_by_name(char* name) {
	client_l aux;
	aux = client_list;
	while (aux != NULL) {
    int name_size = sizeof(aux->name);
		if (strncmp(aux->name, name, name_size) == 0) return aux;
		aux = aux->next;
	}
	return NULL;
}

client_l find_cl_by_id(int id) {
	client_l aux;
	aux = client_list;
	while (aux != NULL) {
		if (id == aux->id) break;
		aux = aux->next;
	}
	return aux;
}

int valid_name(char* name) {
  client_l aux;
  int ret = 1;
	aux = client_list;
	while (aux != NULL) {
    if (strncmp(aux->name, name, LIST_LEN_NAME) == 0) {
      ret = -1;
      break;
    }
		aux = aux->next;
	}
	return ret;
}

int get_descriptor(int id) {
	client_l aux;
  int ret = -1;
	aux = client_list;
	while (aux != NULL) {
		if (aux->id == id) {
      ret = aux->descriptor;
      break;
    }
		aux = aux->next;
	}
	return ret;
}

int get_name(int id, char* buffer) {
  int ret = -1;
  client_l aux = client_list;

  buffer = malloc(sizeof(char) * LIST_LEN_NAME);
  memset(buffer, 0, LIST_LEN_NAME);

	while (aux != NULL) {
		if (aux->id == id) {
      memcpy(buffer, aux->name, LIST_LEN_NAME);
      ret = strlen(buffer);
      break;
    }
		aux = aux->next;
	}
	return ret;
}

int get_status(int id) {
	client_l aux;
  int ret = -1;
	aux = client_list;
	while (aux != NULL) {
		if (aux->id == id) {
      ret = aux->status;
      break;
    }
		aux = aux->next;
	}
	return ret;
}

int get_list(char **buffer) {
  int n = 0;
  int ret = (sizeof(char) * nclients * LIST_LEN_NAME) + 6;
  client_l aux = client_list;
  (*buffer) = (char*) malloc(ret);
  memset((*buffer), 0, ret);
  strncpy((*buffer), "list\n", strlen("list\n"));
  while (aux != NULL) {
    strncat((*buffer), aux->name, LIST_LEN_NAME);
    strncat((*buffer), "\n", 1);
    aux = aux->next;
    n++;
  }
  strncat((*buffer), "\0", 1);
  return ret;
}

void free_list() {
  sem_wait(&client_list_semaphore);
  client_l aux;
  while (client_list != NULL) {
    aux = client_list;
    client_list = client_list->next;
    free(aux);
  }
  sem_post(&client_list_semaphore);
  sem_destroy(&client_list_semaphore);
}
