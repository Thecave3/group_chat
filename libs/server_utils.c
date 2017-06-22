#include "server_utils.h"

#define DATA_BUFFER_LEN 16


int	id_clients;

client_l find_cl(int id) {
	if (sem_wait(&client_list_semaphore)) return NULL;
	client_l aux;
	aux = client_list;
	while (aux != NULL) {
		if (aux->client_id == id) break;
		aux = aux->next;
	}
	if (sem_post(&client_list_semaphore)) return NULL;
	return aux;
}

int find_id_by_name(char* name) {
	if (sem_wait(&client_list_semaphore)) return -1;
	client_l aux;
	aux = client_list;
	while (aux != NULL) {
    int client_name_size = sizeof(aux->client_name);
		if (strncmp(aux->client_name, name, client_name_size) == 0) break;
		aux = aux->next;
	}
	if (sem_post(&client_list_semaphore)) return -1;
	return aux->client_id;
}

int valid_name(char* name) {
  client_l aux;
  int ret = 1;
	aux = client_list;
	while (aux != NULL) {
    if (strncmp(aux->client_name, name, MAX_LEN_NAME) == 0) {
      ret = 0;
      break;
    }
		aux = aux->next;
	}
	return ret;
}

int add_cl(client_l client) {
	if (sem_wait(&client_list_semaphore)) return -1;
  client->client_id = id_clients;
  client->client_status = ONLINE;
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
  if (sem_post(&client_list_semaphore))return -1;
  return 1;
}

int remove_cl(int id) {
  int ret = 0;
	if (sem_wait(&client_list_semaphore)) return -1;
  client_l aux, bux;
  aux = client_list;
  while (aux != NULL) {
    if (aux->client_id == id) {
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
	    nclients--;
      ret++;
      free(bux);
      break;
    }
		aux = aux->next;
    }
  if (sem_post(&client_list_semaphore)) return -1;
  return ret;
}

int send_cl(int sock_desc) {
  int       ret;
	int       list_len;
	int       bytes_send;
	int       data_buffer_len;
	char      data_buffer[MAX_LEN_NAME];
	client_l  aux = client_list;

  list_len = 0;
	bytes_send = 0;

	while (aux != NULL) {
		if (aux->client_status == ONLINE) {
			memset(data_buffer, 0, MAX_LEN_NAME);
			strcat(data_buffer, aux->client_name);
			strcat(data_buffer,"\n\r");
			data_buffer_len = strlen(data_buffer);
			while (bytes_send < data_buffer_len) {
				ret = send(sock_desc, data_buffer + bytes_send, data_buffer_len - bytes_send, 0);
				if (ret == -1 && errno == EINTR) continue;
				if (ret == -1) return -1;
				bytes_send += ret;
			}
			list_len += bytes_send;
			bytes_send = 0;
		}
		aux = aux->next;
	}
	ret = 0;
	while (ret == 0) {
		ret = send(sock_desc, "\0", 1, 0);
		if (ret == -1 && errno == EINTR) continue;
		if (ret == -1) {
			perror("send_cl: error in send");
			return -1;
		}
	}
	list_len ++;
	return list_len;
}
