#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"


int main(int argc, char *argv[]) {
  int sock_desc,ret;
  //separati solo per debug
  //int id_shared_memory,pid,listener;
  input_struct * in_params;
  output_struct * out_params;

  char name[MAX_LEN_NAME];
  //char output[BUF_LEN];

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


  sock_desc = server_connect(name);
  ERROR_HELPER(sock_desc,"Errore connessione al server");

   in_params = malloc(sizeof(input_struct));
   in_params->sock_desc = sock_desc;

   out_params = malloc(sizeof(output_struct));
   out_params->sock_desc = sock_desc;

  /*Creo due thread diversi per gestire stdin e stdout e le operazioni di send e receive dal server*/
  ret = pthread_create(&t_input,NULL,mini_shell,in_params);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_input: ");

  ret = pthread_create(&t_output,NULL,server_output,out_params);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_output: ");



  ret = pthread_join(t_output, &status);
  PTHREAD_ERROR_HELPER(ret,"Errore join out: ");
  ret = pthread_join(t_input, &status);
  PTHREAD_ERROR_HELPER(ret,"Errore join in: ");

  printf("Bye Bye\n");
  return 0;
}
