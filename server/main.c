#include "libs/common.h"

void switching_routine(int client_desc,
		      struct sockaddr_in client_addr);

int main(int argc,
	 char const *argv[]) 
{
	int 	server_desc , client_sock, ret;
	struct 	sockaddr_in server_addr , client_addr;
  
	// Controllo sui valori in input
	if (argc != 2) {
		fprintf(stderr, "usage: central_server: PORT\n");
		exit(EXIT_FAILURE);
	}
	ret = atoi(argv[1]);
	if(ret > 1024 && ret < 65535) {
		fprintf(stderr, "Incorrect port value\n");
		exit(EXIT_FAILURE);
	}
	
	// Inizializzazione porta socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1) {
		fprintf(stderr, "Could not create socket");
    		exit(EXIT_FAILURE);
	}
  	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	if(bind(socket_desc,(struct sockaddr *)&server_addr , sizeof(server)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	listen(socket_desc , MAX_CONN_QUEUE);
  
	// Ciclo sentinella
	while(1) {
    		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    		if (client_sock < 0) {
        		perror("Unable to connect to client");
        		return 1;
    		}
		fprintf(stderr, "Incoming connection");
		
		// Routine di smistamento
		switching_routine(client_desc, client_addr);
  	}
  	exit(EXIT_SUCCESS);
}
void switching_routine(int client_desc, 
		      struct sockaddr_in client_addr)
{
	
	exit();
}
