#include "../libs/common.h"

#define DEFAULT_NAME "thecave3"
#define MAX_LEN_NAME 8

int main(int argc, char *argv[]) {
  int ret;
  int sock;
  struct sockaddr_in server_addr = {0};
  char* name;

  if (argv[1] != NULL)
      name = argv[1];
  else
      name = DEFAULT_NAME;

  printf("Your name is %s\n", name);

  sock = socket(AF_INET,SOCK_STREAM,0);
  ERROR_HELPER(sock,"Errore creazione socket: ");

  server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  ret = connect(sock,(struct sockaddr*) &server_addr,sizeof(struct sockaddr_in));
  ERROR_HELPER(ret,"Errore connect: ");



  return 0;
}
