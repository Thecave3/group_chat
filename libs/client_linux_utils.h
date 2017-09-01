#ifndef CLIENT_LINUX_H
#define CLIENT_LINUX_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "colors.h"
#include "server_protocol.h"

#define CLEAR           	"clear"
#define HELP              "help"
#define MIN_CMD_LEN 4

#define EXIT_SUCCESS 0

#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {               \
  if (cond) {                                                 \
    fprintf(stderr, "%s%s: %s\n",KRED, msg, strerror(errCode));    \
    exit(EXIT_FAILURE);                                     \
  }                                                           \
} while(0)
#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

// Pulisce lo schermo
void clear_screen() {
  printf("%s\e[1;1H\e[2J\n",KNRM);
  printf(">> ");
  ERROR_HELPER(fflush(stdout),"Errore fflush");
}
// Mostra l'elenco completo dei comandi disponibile per l'utente
void display_commands() {
  printf("-----------------LISTA COMANDI---------------\n\n");
  printf(">> Mostra la lista degli utenti connessi: ");
  printf("%s%s%s\n",KGRN,LIST,KNRM);
  printf(">> Richiesta connessione verso un utente: ");
  printf("%s%sNOME_UTENTE%s\n\n",KGRN,CONNECT,KNRM);
  printf(">> Disconnessione dal server ed uscita dal programma: ");
  printf("%s%s%s\n",KGRN,QUIT,KNRM);
  printf(">> Pulisci schermo: ");
  printf("%s%s%s\n\n",KGRN,CLEAR,KNRM);
  printf(">> Mostra questa lista: ");
  printf("%s%s%s\n",KGRN,HELP,KNRM);
  printf(">> ");
  ERROR_HELPER(fflush(stdout),"Errore fflush");
}

#endif
