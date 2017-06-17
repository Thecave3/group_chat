#include "server_utils.h"

#define DATA_BUFFER_LEN 16


int	id_clients;                                                                  // Contatore che genera id dei client

int add_cl(client_l client) {
	if (sem_wait(&client_list_semaphore)) return 0;                                // Mi assicuro di essere l'unico ad operare sulla SCL
	client->client_id = id_clients;                                                // Assegno l'id al client
	client->client_status = ONLINE;                                                // Setto lo stato ad online
	if (nclients == 0) {                                                           // Se il numero di client connessi è 0
		client_list = client;                                                        // Setto la struttura sia come testa dell'SCL
		last_client = client;                                                        // che come coda
	}
	else {
		client->prev = last_client;                                                  // Altrimenti pongo l'ultimo elemento della coda come
		last_client->next = client;                                                  // precedente a quello da inserire e viceversa
		last_client = client;                                                        // Etichetto il nuovo elemento come coda della lista
	}
	nclients++;                                                                    // Incremento il contatore del numero di client connessi
	id_clients++;                                                                  // E quello che genera gli id unici per ogni client
	if (sem_post(&client_list_semaphore))return 0;                                 // Segnalo agli altri thread che ho finito di operare sulla SCL
	return 1;                                                                      // Ritorno 1 in caso di successo
}

int remove_cl(int id) {
  int ret = -1;
	if (sem_wait(&client_list_semaphore)) return -1;                               // Mi assicuro di essere l'unico a operare sulla SCL
	client_l aux, bux;                                                             // Creo due puntatori per gestire la lista
	aux = client_list;                                                             // Faccio puntare aux alla testa della lista
	while (aux != NULL) {                                                          // Fin tanto che questo è diverso da NULL
		if (aux->client_id == id) {                                                  // Se aux è l'elemento da eliminare
			bux = aux;                                                                 // Faccio puntare anche bux allo stesso elemento di aux
			if (aux->next == NULL && aux->prev == NULL) {                              // Se sia l'elemento precedente che il successivo di aux
				client_list = NULL;                                                      // sono uguali a NULL aux è l'unico elemento della SCL
				last_client = NULL;                                                      // pertanto resetto sia last_client che client_list
			}
			else if (aux->prev == NULL) {                                              // Se solo il precedente è NULL allora aux è in testa alla
				client_list = aux->next;                                                 // SCL quindi setto il successivo come capo della SCL e
				client_list->prev = NULL;                                                // cancello da quest'ultimo il riferimento al precedente
			}
			else if (aux->next == NULL) {                                              // Se solo il successivo è NULL allora aux è in coda alla
				last_client = aux->prev;                                                 // SCL quindi setto il precedente come coda della SCL e
				last_client->next = NULL;                                                // cancello da quest'ultimo il riferimento al successivo
			}
			else {                                                                     // Altrimenti aux si trova in mezzo alla lista quindi faccio
				aux = bux->prev;                                                         // puntare aux all'elemento precedente e pongo come riferimento
				aux->next = bux->next;                                                   // all'elemento successivo di aux il riferimento successivo
				aux = bux->next;                                                         // di bux poi faccio puntare aux all'elemento successivo
				aux->prev = bux->prev;                                                   // di bux e pongo come riferimento le'emento precedente
			}
	    nclients--;                                                                // Decremento il numero di client connessi
      ret++;                                                                     // Setto ret a 0 per indicare un match della funzione
			free(bux);                                                                 // Libero la zona di memoria puntata da bux
			break;                                                                     // Interrompo il ciclo
		}
		aux = aux->next;                                                             // Scorro la SCL
	}
	if (sem_post(&client_list_semaphore)) return -1;                               // Segnalo agli altri thread che ho finito di operare sulla SCL
	return ret;                                                                    // Ritorno ret
}

int 	send_cl(int sock_desc) {
	int       ret;
	int       list_len;
	int       bytes_send;
	int       data_buffer_len;
	char      data_buffer[DATA_BUFFER_LEN];
	client_l  aux = client_list;

  list_len = 0;
	bytes_send = 0;

	while (aux != NULL) {
		if (aux->client_status == ONLINE) {
			fprintf(stderr, "%s\n", aux->client_name);
			memset(data_buffer, 0, DATA_BUFFER_LEN);
			sprintf(data_buffer,"%d" ,aux->client_id);
			strcat(data_buffer, "\n");
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

client_l find_cl(int id) {
  int ret = -1;
	if (sem_wait(&client_list_semaphore)) return NULL;                             // Mi assicuro di essere l'unico a operare sulla SCL
	client_l aux, bux;                                                             // Creo due puntatori per gestire la lista
	aux = client_list;                                                             // Faccio puntare aux alla testa della lista
	while (aux != NULL) {                                                          // Fin tanto che questo è diverso da NULL
		if (aux->client_id == id) break;
		aux = aux->next;                                                             // Scorro la SCL
	}
	if (sem_post(&client_list_semaphore)) return NULL;                             // Segnalo agli altri thread che ho finito di operare sulla SCL
	return aux;                                                                    // Ritorno ret
}

int invalid_name(char* name) {
  int ret = 0;
  if (sem_wait(&client_list_semaphore)) return -1;                               // Mi assicuro di essere l'unico a operare sulla SCL
  client_l aux, bux;                                                             // Creo due puntatori per gestire la lista
  aux = client_list;                                                             // Faccio puntare aux alla testa della lista
  while (aux != NULL) {                                                          // Fin tanto che questo è diverso da NULL
		if (strcmp(aux->client_name, name) == 0) {                                   // Se aux è l'elemento da eliminare
	  	ret++;
    }
		aux = aux->next;                                                             // Scorro la SCL
	}
	if (sem_post(&client_list_semaphore)) return -1;                               // Segnalo agli altri thread che ho finito di operare sulla SCL
	return ret;                                                                    // Ritorno ret
}
