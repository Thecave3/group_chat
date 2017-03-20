#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

#define DEFAULT_PORT "5678"

int main(int argc, char *argv[]) {
  int sock,id_shared_memory,pid,ret,listener;

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

  /* Armare il signale*/
  sigfillset (&block_mask);
  usr_action.sa_handler = tactical_change;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  ret = sigaction (SIGUSR1, &usr_action, NULL);
  ERROR_HELPER(ret,"Errore armamento segnale SIGUSR1: ");

  printf("Benvenuto %s", name);
  printf("Provo a connettermi al server...\n");


  sock = server_connect(name, port);
  ERROR_HELPER(sock,"Errore connssione al server: ");

  //0660 utente del gruppo e proprietari hanno facoltà di modificarla e leggerla, gli altri no
  id_shared_memory = shmget(IPC_PRIVATE,CLIENT_SIZE*MAX_CLIENTS,IPC_CREAT|IPC_EXCL|0660);
  ERROR_HELPER(id_shared_memory,"Errore creazione shared memory: ");

  pid = fork();
  ERROR_HELPER(pid,"Errore fork: ");
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
    struct sockaddr_in addr_listener = {0};
    struct sockaddr_in client_addr = {0};
    int addr_len = sizeof(struct sockaddr_in);
    int chatter;
    listener = socket(AF_INET,SOCK_STREAM,0);
    ERROR_HELPER(listener,"Errore creazione server: ");
    addr_listener.sin_family = AF_INET;
    addr_listener.sin_port = htons(atoi(port));
    addr_listener.sin_addr.s_addr= INADDR_ANY;
    ret = bind(listener,(struct sockaddr*) &addr_listener,addr_len);
    ERROR_HELPER(ret,"Errore bind server: ");
    ret = listen(listener,CLIENT_QUEUE);
    ret = fcntl(listener,F_SETFL,O_NONBLOCK);
    ERROR_HELPER(ret,"Errore sblocco server bloccante: ");


    while(parent_status){
      chatter = accept(listener,(struct sockaddr*)&client_addr, (socklen_t*) &addr_len);
      if (chatter == -1){
        if(errno== EINTR ||errno == EAGAIN)
          continue;
        else
          ERROR_HELPER(chatter, "Errore accept: ");
      }
      //ret = close(chatter);
      //ERROR_HELPER(ret,"Errore chiusura chatter: ");
    }
    ret = close(listener);
    ERROR_HELPER(ret,"Errore chiusura listener:");


    kill(pid,SIGTERM);

    //invia messaggio di non disponibile al server e apre un nuovo terminale e reindirizzandoci stdin e stdout
    ret = send_message(sock,STOF,sizeof(STOF));

    listener= end_end_chat(id_shared_memory);


    //e da qui va lanciato il magico terminale che crea la chat


    printf("esco\n");
    command_request(QUIT,sock,id_shared_memory);
  }
  return 0;
}
