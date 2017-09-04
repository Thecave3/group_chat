#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "main_routine.h"

#define HELP "usage %s ACTION OPTION\nACTION:\n\t--start: start the server\n\t--kill:  kill the server\n"

int 	main(int argc, char const *argv[]) {
  FILE     *fp;                                                                   // Filepointer per il salvataggio del risultato della popopen()
  pid_t    pid;                                                                   // Variabile per il salvataggio del pid del processo in esecuzione
  int      check;                                                                 // Variabile per il controllo di errori da parte dell'utente
  int      check_pid;
  int      i;                                                             // Variabile per il confronto dei pid;
  char     buffer[64];                                                            // Buffer di allocazione per le varie stringhe usate nel programma

  pid = getpid();                                                                 // Ottengo il pid del processo corrente
  check = 0;                                                                      // Resetto la variabile check
  sprintf(buffer, "ps -ef | grep %s | grep -v grep | awk '{print $2}'",           // Sposto nel buffer la stringa contenente il comando con cui
                  argv[0]);                                                       // ottengo il pid della prima istanza server avviata
  fp = popen(buffer, "r");                                                        // Eseguo il comando tramite la funzione popen()
  if (fp == NULL) {                                                               // Se fp == NULL l'operazione non è andata a buon fine
    perror("Popen() Fail");
    exit(EXIT_FAILURE);
  }
  fgets(buffer, sizeof(buffer)-1, fp);                                            // Sposto in buffer il contenuto di fp
  check_pid = atoi(buffer);                                                       // Converto il contenuto di fp (pid) in un intero e lo salvo in check_pid
  pclose(fp);                                                                     // Chiudo fp

  for (i = 1; i < argc; i++) {                                                // Scorro gli elementi passati come parametri del programma da terminale
    if(strcmp(argv[i], "--start") == 0) {                                         // Se uno dei parametri è uguale a "--start"
      if (pid != check_pid) {                                                     // Se il pid del programma è diverso dall'ultima istanza del server avviata
        fprintf(stderr, "Server already started\n");                              // informo l'utente che il server è già stato avviato
        check++;                                                                  // Incremento la variabile check
        break;                                                                    // Interrompo il ciclo
      }
      pid = fork();                                                               // Se non è già stata avviata alcuna istanza del server creo un processo figlio
      if (pid < 0) {                                                              // Se pid < 0 l'operazione non è andata a buon fine
        perror("Fork() Fail");                                                    // Stampo un messaggio di errore
        exit(EXIT_FAILURE);                                                       // Termino il processo
      }
      if (pid > 0) main_routine(argc,argv);                                     // Se il pid è maggiore di 0 sono nel processo figlio quindi avvio la routine del server
      check++;                                                                    // Incremento la variabile check
      break;                                                                      // Interrompo il ciclo
    }
    else if(strcmp(argv[i], "--kill") == 0)  {                                    // Se uno dei parametri è uguale a "--kill"
      pid = atoi(buffer);                                                         // Prendo il pid all'interno del buffer
      if (pid == getpid()) {                                                      // Se è uguale a quello dell'istanza corrente del programma significa che questa è unica
        fprintf(stderr, "Server not running\n");                                  // Avverto l'utente di ciò
        check++;                                                                  // Incremento la variabile check
        break;                                                                    // Termino il programma
      }
      kill(pid, SIGINT);                                                          // Invia il segnale di terminazione all'altra istanza del server in esecuzione
      check++;                                                                    // Incremento la variabile check
      break;                                                                      // Termino il ciclo
    }
    else if(strcmp(argv[i], "--help") == 0) {                                     // Se uno dei parametri è uguale a "--kill"
      fprintf(stderr,  HELP, argv[0]);                                            // Stampa la lista dei comandi disponibili per il server
      check++;                                                                    // Incremento la variabile check
      break;                                                                      // Interrompo il ciclo
    }
  }
  if (check == 0) fprintf(stderr, "Type: %s --help\n", argv[0]);                  // Verifico se si è verificato un errore da parte dell'utente
  exit(EXIT_SUCCESS);                                                             // Se è accaduto stampo un messaggio di errore
}
