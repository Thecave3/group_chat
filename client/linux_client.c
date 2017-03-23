#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

int main(int argc, char *argv[]) {
  int sock,id_shared_memory,pid,ret,listener;

  char name[MAX_LEN_NAME];
  char command[BUF_LEN];
  pthread_t t_input,t_output;

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


  /* Armare il signale -- POTREBBE ESSERE DEPRECATA

  struct sigaction usr_action;
  sigset_t block_mask;

  sigfillset (&block_mask);
  usr_action.sa_handler = tactical_change;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  ret = sigaction (SIGUSR1, &usr_action, NULL);
  ERROR_HELPER(ret,"Errore armamento segnale SIGUSR1: ");
  */

  printf("Benvenuto %s", name);
  printf("Provo a connettermi al server...\n");


  sock = server_connect(name);
  ERROR_HELPER(sock,"Errore connssione al server: ");

  //0660 utente del gruppo e proprietari hanno facoltà di modificarla e leggerla, gli altri no
  id_shared_memory = shmget(IPC_PRIVATE,CLIENT_SIZE*MAX_CLIENTS,IPC_CREAT|IPC_EXCL|0660);
  ERROR_HELPER(id_shared_memory,"Errore creazione shared memory: ");

  /*Creo due thread diversi per gestire stdin e stdout e le operazioni di send e receive dal server*/
  ret = pthread_create(&t_input,NULL,mini_shell,NULL);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_input: ");

  ret = pthread_create(&t_output,NULL,/*funzione da definire*/,NULL);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_output: ");


  return 0;
}
