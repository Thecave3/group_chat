#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include "../libs/logger.h"
#include "../libs/colors.h"
#include "../libs/server_utils.h"
#include "../libs/server_protocol.h"

logger_t* main_logger;
#define LOG_PATH ".logs"

void *init_client_routine(void* arg);
void 	goodbye ();

int main(int argc, char const *argv[]) {
	int									server_desc , client_desc, client_addr_len, ret;
	char 								buffer[128];
	struct sockaddr_in	server_addr , client_addr;
	struct stat st = {0};

	atexit(goodbye);
  signal(SIGINT, exit);
	server_init(&server_desc, &server_addr);

	if (stat(LOG_PATH, &st) == -1) {
		mkdir(LOG_PATH, 0700);
	}

	strcpy(buffer, LOG_PATH);
	strcat(buffer, "/");
	strcat(buffer, "Server_log");
	strcat(buffer, " - ");
	strcat(buffer, get_time());


	main_logger = new_log(buffer, O_WRONLY | O_CREAT | O_APPEND, 0666);

	fprintf(stderr, "%s: Server started\n", get_time());
	write_log(main_logger, "%s: Server started\n", get_time());

 	// Ciclo sentinella
	client_addr_len = sizeof(client_addr);
	while(1) {
		client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) {
			if (DEBUG) perror("client_desc: error in accept");
			fprintf(stderr, "Impossibile connettersi al client\n");
			continue;
    }
		client_l thread_arg = malloc(sizeof(client_t));
		thread_arg->client_desc = client_desc;
		thread_arg->next = NULL;
		thread_arg->prev = NULL;
		sprintf(thread_arg->client_ip,
			"%d.%d.%d.%d",
			(int)(client_addr.sin_addr.s_addr&0xFF),
			(int)((client_addr.sin_addr.s_addr&0xFF00)>>8),
			(int)((client_addr.sin_addr.s_addr&0xFF0000)>>16),
			(int)((client_addr.sin_addr.s_addr&0xFF000000)>>24));
		pthread_t* init_client_thread = malloc(sizeof(pthread_t));
		ret = pthread_create(init_client_thread, NULL, init_client_routine,(void*) thread_arg);
		if (ret != 0) {
			if (DEBUG) fprintf(stderr, "init_client_thread: error in ptrhead_create: %s\n", strerror(ret));
			fprintf(stderr, "Impossibile connettersi al client\n");
			continue;
		}
		ret = pthread_detach(*init_client_thread);
		if (ret != 0) {
			if (DEBUG) fprintf(stderr, "init_client_thread: error in ptrhead_detach: %s\n", strerror(ret));
			fprintf(stderr, "Impossibile connettersi al client\n");
			continue;
		}
		memset(&client_addr, 0, sizeof(client_addr));
  }
  exit(EXIT_SUCCESS);
}

void *init_client_routine(void *arg) {
	client_l 	client = (client_l) arg;

	int*			status = &client->client_status;
	int 			ret;
	int 			client_desc = client->client_desc;
	int   		bytes_read = 0;
	int   		query_ret;
	int  		 	query_recv;
	int				client_id = client->client_id;
	char*			client_name = client->client_name;
	char*			client_ip = client->client_ip;
	char			query[5];
	char 			buffer[128];

  while(1) {
    ret = recv(client_desc, client_name + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret == -1) {
			if (DEBUG) perror("recv_message: error in recv");
      pthread_exit(NULL);
    }
    if (ret == 0) {
			if (DEBUG) perror("recv_message: connection closed by client");
      pthread_exit(NULL);
    }
    bytes_read++;
    if (client_name[bytes_read-1] == '\n' ||
				client_name[bytes_read-1] ==  '\0' ||
				bytes_read == 63)
			break;
  }
  client_name[bytes_read-1] = '\0';

	strcpy(buffer, LOG_PATH);
	strcat(buffer, "/");
	strcat (buffer, client_name);
	strcat (buffer, " - ");
	strcat (buffer, get_time());
	strcat (buffer, ".txt");

	logger_t* client_logger = new_log(buffer, O_WRONLY | O_CREAT | O_APPEND, 0666);
	*status = ONLINE;

	write_log(client_logger, "%s\nLog of client %s : %s\n", get_time(), client_ip, client_name);
	write_log(main_logger, "%s: New client connected %s : %s\n", get_time(), client_ip, client_name);
	if (DEBUG) fprintf(stderr, "%s: %s [%s] "KYEL"NEW_CONNECTION"KNRM" => "KGRN"ONLINE"KNRM"\n", get_time(), client_name, client_ip);
	add_cl(client);
	while (1) {
		query_recv = 0;
		memset(query, 0, QUERY_LEN);
	  while(1) {
	    query_ret = recv(client_desc, query + query_recv, 1, 0);
	    if (query_ret == -1 && errno == EINTR)
				continue;
	    if (query_ret == -1) {
				if (DEBUG) perror("recv_message: error in recv");
				pthread_exit(NULL);
	    }
	    if (query_ret == 0) {
				if (DEBUG) perror("recv_message: connection closed by client");
	      pthread_exit(NULL);
	    }
	    query_recv++;
	    if (query[query_recv-1] == '\n' ||
					query[query_recv-1] ==  '\0' ||
					query_recv == QUERY_LEN)
				break;
	  }
	  query[query_recv-1] = '\0';
		if (strcmp(query, "QUIT\0") == 0) {
			remove_cl(client_id);
			write_log(client_logger, "%s: Client disconnected\n", get_time());
			destroy_log(client_logger);
			write_log(main_logger, "%s: Client disconnected %s : %s\n", get_time(), client_ip, client_name);
			if (DEBUG) fprintf(stderr, "%s: %s [%s] "KGRN"ONLINE"KNRM" => "KCYN"CLOSE_CONNECTION"KNRM"\n", get_time(), client_name, client_ip);
			break;
		}
		if (strcmp(query, "STOF\0") == 0) {
			if (*status != OFFLINE) {
				*status = OFFLINE;
				write_log(client_logger, "%s: Change status Online => Offline\n", get_time());
				if (DEBUG) fprintf(stderr,"%s: %s [%s] "KGRN"ONLINE"KNRM" => "KRED"OFFLINE"KNRM"\n", get_time(), client_name, client_ip);
			}
		}
		if (strcmp(query, "STON\0") == 0){
			if (*status != ONLINE) {
				*status = ONLINE;
				write_log(client_logger, "%s: Change status Offline => Online\n", get_time());
				if (DEBUG) fprintf(stderr, "%s: %s [%s] "KRED"OFFLINE"KNRM" => "KGRN"ONLINE"KNRM"\n", get_time(), client_name, client_ip);
			}
		}
		if (strcmp(query, "LIST\0") == 0) {
			send_cl(client_desc);
			write_log(client_logger, "%s: Client asked the list\n", get_time());
			if (DEBUG) fprintf(stderr, "%s: %s [%s] >> "KWHT"LIST"KNRM"\n", get_time(), client_name, client_ip);
		}
	}
	pthread_exit(NULL);
}


void 	goodbye () {
	fprintf(stderr, "\n");
	write_log(main_logger, "%s: Server halted\n", get_time());
	destroy_log(main_logger);
	if (DEBUG) fprintf(stderr, "Svuoto la lista dei client\n");
	while (client_list != NULL) {
		client_l aux = client_list;
		if(DEBUG) fprintf(stderr, "%d %s %s\n",aux->client_id, aux->client_ip, aux->client_name);
		client_list = client_list->next;
		free(aux);
	}
	sem_destroy(&client_list_semaphore);
	fprintf(stderr, "The Server say goodbye!\n");
}
