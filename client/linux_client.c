#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

int main(int argc, char *argv[]) {
  int ret;
  int sock;
  int id_shared_memory;

  char name[MAX_LEN_NAME];
  char command[BUF_LEN];

  if (argv[1] != NULL && strlen(argv[1])<=MAX_LEN_NAME) {
    strcpy(name,argv[1]);
    strcat(name, "\n");
  }
  else if(argv[1] != NULL && strlen(argv[1])>MAX_LEN_NAME){
    printf("%sNome utente troppo lungo, nome utente massimo consentito: %d\n",KRED,MAX_LEN_NAME);
    exit(EXIT_FAILURE);
  }
  else {
    strcpy(name, DEFAULT_NAME);
    strcat(name, "\n");
  }


  //0660 utente del gruppo e proprietari hanno facoltà di modificarla e leggerla, gli altri no
  id_shared_memory = shmget(IPC_PRIVATE,32*MAX_CLIENTS,IPC_CREAT|IPC_EXCL|0660);
  ERROR_HELPER(id_shared_memory,"Errore creazione: ");



  printf("Benvenuto %s", name);

  printf("Provo a connettermi al server...\n");
  //todo usare API di giorgio
  sock = connect_to(SERVER_ADDRESS,SERVER_PORT);

  //inviare nome send
  ret = send_message(sock, name, MAX_LEN_NAME);
  ERROR_HELPER(ret,"Errore invio del nome: ");

  printf("Scrivi \"%s\" per aiuto\n",HELP);


  while (1) {
    printf("Inserisci un comando: ");
    fgets(command,sizeof(command),stdin);
    printf("\n");
    command_request(command,sock,id_shared_memory);
  }
  return 0;
}
