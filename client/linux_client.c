#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"


int main(int argc, char *argv[]) {

  pthread_t t_output;
  output_struct * out_params;
  out_params = malloc(sizeof(output_struct));
  int sock_desc;
  char name[MAX_LEN_NAME];
  char list[MAX_LEN_LIST];

  //Incipit e controlli vari sul nome
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

  //Connessione al server
  sock_desc = server_connect(name);
  ERROR_HELPER(sock_desc,"Errore connessione al server");

  printf("\nConnessione effettuata\n");
  out_params->sock_desc = sock_desc;

  // Creazione thread per gestire l'output
  ret = pthread_create(&t_output,NULL,server_output,out_params);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_output: ");

  //Lancio shell
  mini_shell(sock_desc,list);


  printf("Bye Bye\n");
  return 0;
}
