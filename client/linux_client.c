#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"


int main(int argc, char *argv[]) {

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

  //Lancio shell
  mini_shell(sock_desc,list);


  printf("Bye Bye\n");
  return 0;
}
