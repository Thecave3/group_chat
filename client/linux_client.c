#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

int main(int argc, char *argv[]) {
  int sock,id_shared_memory,pid,status,result;

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

  printf("Benvenuto %s", name);
  printf("Provo a connettermi al server...\n");

  struct sockaddr_in sock_addr = {0};
  sock = server_connect(&sock_addr, name, MAX_LEN_NAME);
  ERROR_HELPER(sock,"Errore connssione al server: ");

  //0660 utente del gruppo e proprietari hanno facoltà di modificarla e leggerla, gli altri no
  id_shared_memory = shmget(IPC_PRIVATE,32*MAX_CLIENTS,IPC_CREAT|IPC_EXCL|0660);
  ERROR_HELPER(id_shared_memory,"Errore creazione shared memory: ");

  pid = fork();
  ERROR_HELPER(pid,"Errore sulla fork: ");
  if (pid == 0) {
    printf("\nScrivi \"%s\" per aiuto\n",HELP);
    command_request(LIST,sock,id_shared_memory);

    while (1) {
      printf("Inserisci un comando: ");
      fgets(command,sizeof(command),stdin);
      printf("\n");
      command_request(command,sock,id_shared_memory);
    }
  }else{
    //printf("sono il padre che aspetta che il figlio finisca\n");


    result = wait(&status);

    //richiesta socket (lancia funzione end_end_chat)
    //dopo aver avuto il socket con la connessione con l'altro client
    //invia messaggio di non disponibile al server e apre un nuovo terminale e reindirizzandoci stdin e stdout
    //ret = send_message(sock_desc,STOF,sizeof(STOF));

    ERROR_HELPER(result,"Errore processo wait");
  }
  return 0;
}
