#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

int main(int argc, char *argv[]) {
  int ret;
  int sock;
  char* name;
  //char list_client[BUF_LEN];
  char command[BUF_LEN];

  if (argv[1] != NULL && strlen(argv[1])<=MAX_LEN_NAME)
    name = argv[1];
  else if(argv[1] != NULL && strlen(argv[1])>MAX_LEN_NAME){
    printf("%sNome utente troppo lungo, nome utente massimo consentito: %d\n",KRED,MAX_LEN_NAME);
    exit(EXIT_FAILURE);
  }else
      name = DEFAULT_NAME;


  printf("Benvenuto %s\n", name);
  sock = connect_to(SERVER_ADDRESS,SERVER_PORT);


  //inviare nome send
  ret = send_message(sock, name, MAX_LEN_NAME);
  ERROR_HELPER(ret,"Errore invio del nome: ");

  printf("Scrivi \"%s\" per aiuto\n",HELP);


  while (1) {
    printf("Inserisci un comando: ");
    fgets(command,sizeof(command),stdin);
    printf("\n");
    command_request(command,sock);
  }
  return 0;
}
