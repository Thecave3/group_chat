#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Set 1 = Attiva il debug
#define DEBUG           0
// Lunghezza delle query al server
#define QUERY_LEN       5
// Address del server
#define SERVER_ADDRESS  "127.0.0.1"
// Porta del server
#define SERVER_PORT     8583
// Lunghezza dei dati da inviare al server
#define PACKET_LEN      16
// Lunghezza massima del nome del client
#define MAX_LEN_NAME    12

// Stati del client
#define OFFLINE   0
#define ONLINE    1

// Effettua una connessione TCP con il server e ritorna il descrittore
// o -1 in caso di errore
int server_connect(char* name, char* port);
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

/*
 * Tutte le funzioni rimandano la gestione dell'errore al livello superiore,
 * per le funzioni server_connect(), download_list() e server_disconnect()
 * è possibile verificare la causa di errore tramite errno tramite metodo
 * standard.
*/

// Interfacce semplificate per send e recv bit a bit (deprecate)
int recv_message(int socket_desc, char* buffer,  int buffer_len);
int send_message(int socket_desc, char* buffer, int buffer_len);

#endif
