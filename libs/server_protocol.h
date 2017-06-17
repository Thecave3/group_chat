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
// Lunghezza delle query al server
#define QUERY_LEN       5
// Address del server
#define SERVER_ADDRESS  "127.0.0.1"
// Porta del server
#define SERVER_PORT     8583
// Lunghezza dei dati da inviare al server
#define PACKET_LEN      16
// Lunghezza massima del nome del client
#define MAX_LEN_NAME    10

// Stati del client
#define OFFLINE   0
#define ONLINE    1

// Effettua una connessione TCP con il server inviando il nome
// Se il nome è già in uso da qualcuno nel server, allora il server ritorna un messaggio di errore e -1
// ritorna il descrittore o -1 in caso di errore
int server_connect(char* name);
// Scarica la lista dei client connessi in stato ONLINE la salva in un
// buffer di dimensione buff_len e ritorna il numero di bytes letti o
// -1 in caso di errore
int download_list(int socket_desc, char* buffer, size_t buff_len);
// Setta lo status del client a ONLINE o OFFLINE e ritorna 1 in caso di
// successo e -1 in caso di errore
int server_status(int sock_desc, int status);
// Disconnette il client dal server e ritorna 1 in caso di successo e
// -1 in caso di errore
int server_disconnect(int sock_desc);
//Invia una richiesta di connessione al client identificato con l'id client_id
//Deve essere una chiamata bloccante, ovvero il flusso di esecuzione del codice deve bloccarsi finchè non si ha risposta
//Ritorna 1 in caso di successo (e parte la chat), 0 in caso di connessione rifiutata da parte dell'altro client,
// 2 in caso di client non trovato, -1 in caso di errore generico
int connect_to(int sock_desc, int client_id);
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
