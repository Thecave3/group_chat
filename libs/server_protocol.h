#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEBUG		0
// Address del server
#define SERVER_ADDRESS  "127.0.0.1"
// Porta del server
#define SERVER_PORT     8583
// Lunghezza massima del nome del client
#define MAX_LEN_NAME    10

#define CONNECT         	    "connect "
#define LIST                  "list\n"
#define QUIT            	    "quit\n"
#define NAME_ALREADY_USED	    "name_used\n"
#define CONNECT_WITH_YOURSELF "connect_with_yourself\n"

// Stati del client
#define OFFLINE   0
#define ONLINE    1

//Utilizzo queste funzioni per far comunicare fra loro i messaggio dei due client
//Se uno dei due client vuole chiudere la conversazione, invierà nella chat tramite send_message la stringa "quit"
//In caso di CTRL-C o di altri segnali il client proverà lo stesso a inviare la stringa quit prima di chiudersi.
//L'unico caso non gestibile di invio al server è l'assenza di connessione.
int recv_message(int socket_desc, char* buffer,  int buffer_len);
int send_message(int socket_desc, char* buffer, int buffer_len);

//Tutte le funzioni rimandano la gestione dell'errore al livello superiore,
//per le funzioni server_connect(), download_list() e server_disconnect()
//è possibile verificare la causa di errore tramite errno tramite metodo
//standard.

#endif
