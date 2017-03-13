/* Client esempio di comunicazione con il server */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include "../libs/common.h"
#include "../libs/server_protocol.h"

int sock_desc;

int main(int argc, char* argv[]) {
  int                 ret;
  char                buffer[256];

  if (argc != 3) {
    fprintf(stderr, "usage: %s NAME\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  if (strlen(argv[1]) > 12) {
    fprintf(stderr, "Nome troppo lungo\n");
    exit(EXIT_FAILURE);
  }
  if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535) {
    fprintf(stderr, "Porta non valida\n");
    exit(EXIT_FAILURE);
  }

  /* Connette il client al server */
  sock_desc = server_connect(argv[1], argv[2]);
  if (sock_desc == -1) exit(EXIT_FAILURE);
  fprintf(stderr, "Connesso al server\n");
  /* Memorizza in buffer la lista dei client disponibili */
  ret = download_list(sock_desc, buffer, sizeof(buffer));
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Ottenuta la Lista:\n%s\n", buffer);
  while (1);
  /* Mettere il client offline */
  ret = server_status(sock_desc, OFFLINE);
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Status Passato ad Offline\n");
  /* Termina la connessione tra server e client */
  ret = server_disconnect(sock_desc);
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Disconnesso dal server\n");
  exit(EXIT_SUCCESS);
}
