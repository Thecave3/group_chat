#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

// Address del server
#define SERVER_ADDRESS /* "79.53.14.217" */ "127.0.0.1"
// Porta del server
#define SERVER_PORT     8583
// Lunghezza massima del nome del client
#define MAX_LEN_NAME    10

#define BUF_LEN 1024

#define CONNECT         	    "connect "
#define LIST                  "list\n"
#define QUIT            	    "quit\n"
#define NAME_ALREADY_USED	    "name_used\n"
#define CONNECT_WITH_YOURSELF "connect_with_yourself\n"
#define CLIENT_NOT_EXIST      "client_not_exist\n"
#define YES                   "yes\n"
#define NO                    "no\n"

// Stati del client
#define BUSY      0
#define ONLINE    1

// Flag del format
#define Z_FLAG 1
#define N_FLAG 2
#define R_FLAG 4

//Utilizzo queste funzioni per far comunicare fra loro i messaggio dei due client
//Se uno dei due client vuole chiudere la conversazione, invierà nella chat tramite send_message la stringa "quit"
//In caso di CTRL-C o di altri segnali il client proverà lo stesso a inviare la stringa quit prima di chiudersi.
//L'unico caso non gestibile di invio al server è l'assenza di connessione.

int recv_message(int socket_desc, char* buffer,  int buffer_len, int flags);
int send_message(int socket_desc, char* buffer, int buffer_len, int flags);

//Tutte le funzioni rimandano la gestione dell'errore al livello superiore

#endif
