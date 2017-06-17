#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"


int main(int argc, char *argv[]) {
  pthread_t t_output;
  output_struct * out_params;
  out_params = malloc(sizeof(output_struct));
  sem_t mutex_sem_stdout;
  int sock_desc,ret,value_sem;
  char name[MAX_LEN_NAME];
  char list[MAX_LEN_LIST];
  char command[BUF_LEN];

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

  //Inizializzo il semaforo
  value_sem = 1;
  ret = sem_init(&mutex_sem_stdout, 0,value_sem);
  ERROR_HELPER(ret,"Errore creazione semaforo: ");

  // Creazione thread per gestire l'output
  out_params->sock_desc = sock_desc;
  out_params->semid = mutex_sem_stdout;
  ret = pthread_create(&t_output,NULL,server_output,out_params);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_output: ");

  //Lancio shell
  printf("\nScrivi \"%s\" per aiuto\n",HELP);
  //command_request(LIST,sock_desc,list);
  while (ret==0){
    ret = sem_wait(&mutex_sem_stdout);
    PTHREAD_ERROR_HELPER(ret,"Errore sem_wait shell: ");
    printf("Inserisci un comando: ");
    fgets(command,sizeof(command),stdin);
    printf("\n");
    printf("faccio post in");
    ret = sem_post(&mutex_sem_stdout);
    PTHREAD_ERROR_HELPER(ret,"Errore sem_post :");
    ret = command_request(command,sock_desc,list);
  }




  ret = pthread_join(t_output, NULL);
  PTHREAD_ERROR_HELPER(ret,"Errore join con l'output thread: ");

  printf("Bye Bye\n");
  return 0;
}
