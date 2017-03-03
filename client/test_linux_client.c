/* Client esempio di comunicazione con il server */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../libs/server_protocol.h"


int main(int argc, char* argv[]) {
  int                 ret;
  int                 sock_desc;
  char                buffer[256];
  struct sockaddr_in  sock_addr;

  signal(SIGINT, exit);

  sock_desc = server_connect(&sock_addr, "Admin\0", sizeof("Adnin\0"));
  if (sock_desc == -1) exit(EXIT_FAILURE);
  fprintf(stderr, "Connesso al server\n");
  ret = download_list(sock_desc, buffer, sizeof(buffer));
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Ottenuta la lista\n");
  ret = server_status(sock_desc, OFFLINE);
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Status Passato ad Offline\n");
  ret = server_disconnect(sock_desc);
  if (ret <= 0) exit(EXIT_FAILURE);
  fprintf(stderr, "Disconnesso dal server\n");
  exit(EXIT_SUCCESS);
  return ret;
}
