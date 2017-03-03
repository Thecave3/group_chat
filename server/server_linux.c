#include "../libs/common.h"
#include "../libs/server_protocol.h"
#include "../libs/server_utils.h"

void *init_client_routine(void* arg);

int main(int argc, char const *argv[]) {
	int									server_desc , client_desc, client_addr_len, ret;
	struct sockaddr_in	server_addr , client_addr;

	atexit(goodbye);
  signal(SIGINT, exit);
	server_init(&server_desc, &server_addr);

	fprintf(stderr, "Server started!\n");

	// Ciclo sentinella
	client_addr_len = sizeof(client_addr);
	while(1) {
		client_desc = accept(server_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
    if (client_desc < 0) {
			if (DEBUG) perror("client_desc: error in accept");
			fprintf(stderr, "Impossibile connettersi al client");
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
			fprintf(stderr, "Impossibile connettersi al client");
			continue;
		}
		ret = pthread_detach(*init_client_thread);
		if (ret != 0) {
			if (DEBUG) fprintf(stderr, "init_client_thread: error in ptrhead_detach: %s\n", strerror(ret));
			fprintf(stderr, "Impossibile connettersi al client");
			continue;
		}
		memset(&client_addr, 0, sizeof(client_addr));
  }
  exit(EXIT_SUCCESS);
}

void *init_client_routine(void *arg) {
	int 			ret;
	int   		bytes_read = 0;
	int   		query_ret;
	int  		 	query_recv;
	char			query[5];

	client_l 	client = (client_l) arg;

  while(1) {
    ret = recv(client->client_desc, client->client_name + bytes_read, 1, 0);
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
    if (client->client_name[bytes_read-1] == '\n' ||
				client->client_name[bytes_read-1] ==  '\0' ||
				bytes_read == 63)
			break;
  }
  client->client_name[bytes_read-1] = '\0';
	client->client_status = ONLINE;
	if (DEBUG) fprintf(stderr, "%s [%s] "KYEL"NEW_CONNECTION"KNRM" => "KGRN"ONLINE"KNRM"\n", client->client_name, client->client_ip);
	add_cl(client);
	while (1) {
		query_recv = 0;
		memset(query, 0, QUERY_LEN);
	  while(1) {
	    query_ret = recv(client->client_desc, query + query_recv, 1, 0);
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
			remove_cl(client->client_id);
			if (DEBUG) fprintf(stderr, "%s [%s] "KGRN"ONLINE"KNRM" => "KCYN"CLOSE_CONNECTION"KNRM"\n", client->client_name, client->client_ip);
			break;
		}
		if (strcmp(query, "STOF\0") == 0) {
			if (client->client_status != OFFLINE) {
				client->client_status = OFFLINE;
				if (DEBUG) fprintf(stderr,"%s [%s] "KGRN"ONLINE"KNRM" => "KRED"OFFLINE"KNRM"\n", client->client_name, client->client_ip);
			}
		}
		if (strcmp(query, "STON\0") == 0){
			if (client->client_status != ONLINE) {
				client->client_status = ONLINE;
				if (DEBUG) fprintf(stderr, "%s [%s] "KRED"OFFLINE"KNRM" => "KGRN"ONLINE"KNRM"\n", client->client_name, client->client_ip);
			}
		}
		if (strcmp(query, "LIST\0") == 0) {
			send_cl(client->client_desc);
			if (DEBUG) fprintf(stderr, "%s [%s] >> "KWHT"LIST"KNRM"\n", client->client_name, client->client_ip);
		}
	}
	pthread_exit(NULL);
}
