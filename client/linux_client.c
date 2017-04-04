#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"

int main(int argc, char *argv[]) {
  int sock,id_shared_memory,pid,ret,listener;

  char name[MAX_LEN_NAME];
  char command[BUF_LEN];
  pthread_t t_input,t_output;

  if (argv[1] != NULL && strlen(argv[1])<=MAX_LEN_NAME) {
    strncpy(name,argv[1],strlen(argv[1]));
    strcat(name, "\n");
  }
  else if(argv[1] != NULL && strlen(argv[1])>MAX_LEN_NAME){
    printf("%sNome utente troppo lungo, nome utente massimo consentito: %d\n",KRED,MAX_LEN_NAME);
    exit(EXIT_FAILURE);
  }
  else {
    strncpy(name, DEFAULT_NAME,strlen(DEFAULT_NAME));
    strcat(name, "\n");
  }


  printf("Benvenuto %s", name);
  printf("Provo a connettermi al server...\n");


  sock = server_connect(name); //devo aspettare che giorgio la cambi
  ERROR_HELPER(sock,"Errore connssione al server: ");


  /*Creo due thread diversi per gestire stdin e stdout e le operazioni di send e receive dal server*/
  ret = pthread_create(&t_input,NULL,mini_shell,NULL);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_input: ");

  ret = pthread_create(&t_output,NULL,/*funzione da definire*/,NULL);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_output: ");



  return 0;
}
