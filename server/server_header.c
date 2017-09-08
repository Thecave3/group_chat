
/* Routine di gestione dei client */
void*	client_routine(void *arg) {



  client-> start_connection = 0;
  bytes_read = 0;
  memset(data, 0, MAX_LEN_NAME);

  // Recupero il nome da attribuire al client
  while (bytes_read < MAX_LEN_NAME) {
    ret = recv(client_desc, data + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      remove_cl(*client_id);
      pthread_exit(NULL);
    }
    if (ret == 0) {
      remove_cl(*client_id);
      pthread_exit(NULL);
    }
    bytes_read++;
    if (data[bytes_read-1] == '\n' ||
	      data[bytes_read-1] == '\r' ||
	      data[bytes_read-1] == '\0')
      break;
  }
  data[bytes_read-1] = '\0';

  memcpy(client_name, data, bytes_read);

  bytes_send = 0;
  // Verifico se esiste gà un client con tale nome
  if (valid_name(client_name) <= 0) {
    query_size = strlen(NAME_ALREADY_USED);
    memset(data, 0, MAX_DATA_LEN);
    memcpy(data, NAME_ALREADY_USED, query_size);
    while (bytes_send < query_size) {
      ret = send(client_desc, data + bytes_send, 1, 0);
      if (ret == -1 && errno == EINTR) continue;
      if (ret == -1) {
        remove_cl(*client_id);
        pthread_exit(NULL);
      }
      bytes_send++;
    }
    pthread_exit(NULL);
  }

  *partner_desc = 0;
  add_cl(client);



  while (1) {
	// Recupero i messaggi dal client
    bytes_read = 0;
	if (FQUIT_flag == 0) {
      memset(data, 0, MAX_DATA_LEN);
      while (bytes_read < MAX_DATA_LEN) {
        ret = recv(client_desc, data + bytes_read, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        if (ret == 0) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_read++;
        if (data[bytes_read-1] == '\n' ||
	        data[bytes_read-1] == '\r' ||
	        data[bytes_read-1] == '\0')
          break;
      }
      data[bytes_read-1] = '\n';
    }

    quit_flag = 0;

    // QUIT: Chiudo la connessione con il client e lo rimuovo dalla client_list
    if (strncmp(data, QUIT, sizeof(QUIT)) == 0) {
      fprintf(stderr, "Client %s disconnected\n", client_name);
      remove_cl(*client_id);
      pthread_exit(NULL);
    }

    // LIST: Invio la client_list connessi al client;
    else if (strncmp(data, LIST, sizeof(LIST)) == 0) {
      fprintf(stderr, "Client %s requests client list\n", client_name);
      bytes_send = 0;
      query_size = strlen(LIST);
      while (bytes_send < query_size) {
        ret = send(client_desc, data + bytes_send, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_send++;
      }
      send_cl(client->client_desc);
      continue;
    }

    // YES: Ricezione ack "accetto richiesta di connessione"
    else if (strncmp(data, YES, sizeof(YES)) == 0 && *partner_desc != 0) {
	  start_connection = &client->start_connection;
      fprintf(stderr, "Client %s accept connection\n",client_name);
      set_status(*client_id, OFFLINE);
      bytes_send = 0;
      query_size = strlen(YES);
      *start_connection = 2;
      while (bytes_send < query_size) {
        ret = send(*partner_desc, data + bytes_send, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_send++;
      }
      while(1) {
		int message_size;
		bytes_read = 0;
		memset(data, 0, MAX_DATA_LEN);
		while (bytes_read < MAX_DATA_LEN) {
          ret = recv(client_desc, data + bytes_read, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          if (ret == 0) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_read++;
          if (data[bytes_read-1] == '\n') break;
        }
		message_size = strlen(data);
		if(*start_connection == 3) {
		  quit_flag = 1;
	      if (strncmp(data, YES, sizeof(YES)) == 0) break;
	      else if (strncmp(data, NO, sizeof(NO)) == 0) break;
	      else {
			*partner_desc = 0;
	        break;
		  }
		}
		fprintf(stderr,"%s: %s",client_name, data);
		bytes_send = 0;
        while (bytes_send < message_size) {
          ret = send(*partner_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
        }
        if(strcmp(data,QUIT) == 0) {
	      *partner_desc = 0;
	      *start_connection = 3;
	      set_status(*client_id, ONLINE);
	      set_status(client->partner_id, ONLINE);
	      break;
		}
      }
    }

    // NO: Ricezione ack "rifiuto richiesta di connessione"
    else if (strncmp(data, NO, sizeof(NO)) == 0 && *partner_desc != 0) {
      start_connection = &client->start_connection;
      fprintf(stderr, "Client %s refuse connection\n",client_name);
      bytes_send = 0;
      query_size = strlen(NO);
      while (bytes_send < query_size) {
        ret = send(*partner_desc, data + bytes_send, 1, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) {
          remove_cl(*client_id);
          pthread_exit(NULL);
        }
        bytes_send++;
      }
      *partner_desc = 0;
      *start_connection = 1;
    }

    // CONNECT nome: richiesta di connessione verso un altro utente

      data[bytes_read - 1] = '\0';
      char* request_name = data + sizeof(CONNECT) - 1;
      client_l aux = find_cl_by_name(request_name);
      // Se il nome comunicato non è presente nella lista lo segnalo al client
      if (aux == NULL || aux->client_status == OFFLINE) {
        bytes_send = 0;
        query_size = strlen(CLIENT_NOT_EXIST);
        memset(data, 0, MAX_DATA_LEN);
        memcpy(data, CLIENT_NOT_EXIST, query_size);
        while (bytes_send < query_size) {
          ret = send(client_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
        }
        fprintf(stderr, "Client %s connection error: %s", client_name, data);
        continue;
      }
      // Se il client prova una connessione verso se stesso lo segnalo al client
      else if (aux->client_id == *client_id) {
        bytes_send = 0;
        query_size = strlen(CONNECT_WITH_YOURSELF);
        memset(data, 0, MAX_DATA_LEN);
        memcpy(data, CONNECT_WITH_YOURSELF, query_size);
        while (bytes_send < query_size) {
          ret = send(client_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
        }
        fprintf(stderr, "Client %s connection error: %s", client_name, data);
        continue;
      }
      // Altrimenti inoltro al client la richiesta di connessione
      else {
        aux->partner_desc = client_desc;
        aux->partner_id = *client_id;
        set_status(*client_id, OFFLINE);
        set_status(aux->client_id, OFFLINE);
        *partner_desc = aux->client_desc;
        start_connection = &aux->start_connection;
        bytes_send = 0;
        query_size = strlen(CONNECT);
        memset(data, 0, MAX_DATA_LEN);
        memcpy(data, CONNECT, query_size);
        strncat(data, client_name, MAX_LEN_NAME);
        strncat(data, "\n", 1);
        while (1) {
          ret = send(*partner_desc, data + bytes_send, 1, 0);
          if (ret == -1 && errno == EINTR) continue;
          if (ret == -1) {
            remove_cl(*client_id);
            pthread_exit(NULL);
          }
          bytes_send++;
          if (data[bytes_send-1] == '\n') break;
        }
        fprintf(stderr, "Client %s request connection with %s\n", client_name, aux->client_name);
        while(1) {
		  if (*start_connection == 1) {
		    set_status(*client_id, ONLINE);
			set_status(aux->client_id, ONLINE);
			*start_connection = 0;
			break;
		  }
		  if (*start_connection == 2) {
		    int message_size;
		    bytes_read = 0;
		    memset(data, 0, MAX_DATA_LEN);
		    while (bytes_read < MAX_DATA_LEN) {
              ret = recv(client_desc, data + bytes_read, 1, 0);
              if (ret == -1 && errno == EINTR) continue;
              if (ret == -1) {
                remove_cl(*client_id);
                pthread_exit(NULL);
              }
              if (ret == 0) {
                remove_cl(*client_id);
                pthread_exit(NULL);
              }
              bytes_read++;
              if (data[bytes_read-1] == '\n') break;
            }
		    message_size = strlen(data);
		    if(*start_connection == 3) {
	          quit_flag = 1;
	          if (strncmp(data, YES, sizeof(YES)) == 0) break;
	          else if (strncmp(data, NO, sizeof(NO)) == 0) break;
	          else {
			    *partner_desc = 0;
	            break;
		      }
		    }
		    fprintf(stderr,"%s: %s",client_name, data);
		    bytes_send = 0;
            while (bytes_send < message_size) {
              ret = send(*partner_desc, data + bytes_send, 1, 0);
              if (ret == -1 && errno == EINTR) continue;
              if (ret == -1) {
                remove_cl(*client_id);
                pthread_exit(NULL);
              }
              bytes_send++;
            }
            if(strcmp(data,QUIT) == 0){
			  *partner_desc = 0;
	          quit_flag = 0;
	          set_status(*client_id, ONLINE);
			  set_status(aux->client_id, ONLINE);
			  *start_connection = 3;
	          break;
            }
          }
	    }
      }
    }
  }
  pthread_exit(NULL);
}
