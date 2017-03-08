/* Client esempio di comunicazione con il server */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include "../libs/server_protocol.h"

int sock_desc;

void goodbye() {
  /* Termina la connessione tra server e client */
  int ret = server_disconnect(sock_desc);
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Disconnesso dal server\n");
}

int main(int argc, char* argv[]) {
  int                 ret;
  char                buffer[256];
  struct sockaddr_in  sock_addr;

  atexit(goodbye);
  signal(SIGINT, exit);

  if (argc != 2) {
    fprintf(stderr, "usage: %s NAME\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  strcpy(buffer, argv[1]);
  buffer[strlen(buffer)] = '\0';
  /* Connette il client al server */
  sock_desc = server_connect(&sock_addr, buffer, sizeof(buffer));
  if (sock_desc == -1) exit(EXIT_FAILURE);
  fprintf(stderr, "Connesso al server\n");
  /* Memorizza in buffer la lista dei client disponibili */
  ret = download_list(sock_desc, buffer, sizeof(buffer));
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Ottenuta la Lista:\n%s\n", buffer);
  /* Mettere il client offline */
  ret = server_status(sock_desc, OFFLINE);
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Status Passato ad Offline\n");
  while (1);
}
