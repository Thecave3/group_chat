#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

#define DEFAULT_PORT "5678"

int main(int argc, char *argv[]) {
  int sock,id_shared_memory,pid,status,ret,listener;

  char name[MAX_LEN_NAME];
  char command[BUF_LEN];
  char port[5];

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

  if (argv[2]!= NULL && strlen(argv[2])<=4) {
    strcpy(port,argv[2]);
    strcat(port,"\n");
  }else{
    printf("%sAssegno automaticamente la porta %s %s\n",KNRM,DEFAULT_PORT,KNRM);
    strcpy(port, DEFAULT_PORT);
    strcat(port, "\n");
  }

  struct sigaction usr_action;
  sigset_t block_mask;
  pid_t child_id;

  /* Establish the signal handler. */
  sigfillset (&block_mask);
  usr_action.sa_handler = tactical_change;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  sigaction (SIGUSR1, &usr_action, NULL);

  printf("Benvenuto %s", name);
  printf("Provo a connettermi al server...\n");


  sock = server_connect(name, port);
  ERROR_HELPER(sock,"Errore connssione al server: ");

  //0660 utente del gruppo e proprietari hanno facoltà di modificarla e leggerla, gli altri no
  id_shared_memory = shmget(IPC_PRIVATE,32*MAX_CLIENTS,IPC_CREAT|IPC_EXCL|0660);
  ERROR_HELPER(id_shared_memory,"Errore creazione shared memory: ");

  pid = fork();
  ERROR_HELPER(pid,"Errore sulla fork: ");
  if (pid == 0) {
    printf("\nScrivi \"%s\" per aiuto\n",HELP);
    command_request(LIST,sock,id_shared_memory);

    while (parent_status) {
      printf("Inserisci un comando: ");
      fgets(command,sizeof(command),stdin);
      printf("\n");
      command_request(command,sock,id_shared_memory);
    }
  }else{
    /*struct sockaddr* addr_listener = {0};
    listener = socket(AF_INET,SOCK_STREAM,0);
    ERROR_HELPER(listener,"Errore creazione server: ");
    addr_listener.sin_family = AF_INET;
    addr_listener.sin_port = htons(port);
    addr_listener.sin_addr.s_addr= INADDR_ANY;
    ret = bind(listener,addr_listener,sizeof(addr_listener));
    ERROR_HELPER(ret,"Errore bind server: ");
    ret = fcntl(listener,F_SETFL,O_NONBLOCK);
    ERROR_HELPER(ret,"Errore sblocco server bloccante: ");*/

    //richiesta socket (lancia funzione end_end_chat)
    //dopo aver avuto il socket con la connessione con l'altro client
    //invia messaggio di non disponibile al server e apre un nuovo terminale e reindirizzandoci stdin e stdout
    //ret = send_message(sock_desc,STOF,sizeof(STOF));

    while(parent_status){
      //printf("dormo\n");
      sleep(1);
    }
    printf("SONO TORNATO MERDE\n");

    listener= end_end_chat(id_shared_memory);

    //e da qui va lanciato il magico terminale che crea la chat

    ret = wait(&status);
    ERROR_HELPER(ret,"Errore processo wait");
  }
  return 0;
}
