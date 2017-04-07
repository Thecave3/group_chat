#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>


#include "colors.h"
#include "stringer.h"
#include "server_protocol.h"
#include "common.h"

#define LIST "list"
#define CONNECT "connect"
#define HELP "help"
#define CLEAR "clear"
#define QUIT "quit"
#define MIN_CMD_LEN 4
#define CLIENT_SIZE 32
#define CLIENT_QUEUE 1

#define STON "STON\0"
#define STOF "STOF\0"


#define EXIT_SUCCESS 0


#define DEFAULT_NAME "thecave3"

#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {               \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)
#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

//status threads
void * status;


typedef struct{
  int sock_desc;
  } input_struct;


typedef struct{
    int sock_desc;
  } output_struct;


void clear_screen() {
    printf("%s\e[1;1H\e[2J\n",KNRM );
}



void display_commands() {
  printf("-----------------LISTA COMANDI---------------\n\n");
  printf(">> %s",LIST);
  printf(" :Mostra la lista degli utenti connessi\n\n");
  printf(">> %s",CONNECT);
  printf(" NOME_UTENTE : Richiesta connessione verso un utente \n\n");
  printf(">> %s",QUIT);
  printf(" : Disconnessione dal server ed uscita dal programma.\n\n");
  printf(">> %s",CLEAR);
  printf(" : Pulisci schermo\n\n");
  printf(">> %s",HELP);
  printf(" : Mostra questa lista\n\n");
}


//Gestisce l'input della chat chat
void end_end_chat(int sock_desc){

  return;

}

void command_request(char* buffer,int sock_desc) {
  char* user;
  char list[MAX_LEN_LIST];
  int ret;
  for(int i =0; i<MIN_CMD_LEN;i++){
    if(buffer[i]=='\0'){
      printf("%sComando errato, inserire \"help\" per maggiori informazioni\n",KRED);
      printf("%s\n",KNRM);
      return;
      }
    }

    if (strncmp(buffer,HELP,strlen(HELP))==0) {
      display_commands();
    }else if (strncmp(buffer,LIST,strlen(LIST))==0) {
      printf("Lista utenti connessi:\n");
      ret = download_list(sock_desc,list, sizeof(list));
      ERROR_HELPER(ret,"Errore download lista: ");
      printf("%s\n",list);
      // la lista deve essere salvata da qualche parte
    }else if (strncmp(buffer,QUIT,strlen(QUIT))==0){
      printf("Chiusura connessione in corso... Bye Bye\n");
      server_disconnect(sock_desc);
      /* Armare precedentemente un segnale che chiuda l'altro thread una volta che viene premuto quit
      pthread_exit(&status);*/
    }else if (strncmp(buffer,CLEAR,strlen(CLEAR))==0) {
      clear_screen();
    }else if (strncmp(buffer,CONNECT,strlen(CONNECT))==0) {
      user = subString(buffer,strlen(CONNECT)+1);
      printf("Hai scritto il comando connect verso %s",user);
      int client_id = 221; // solo per debug
        ret = connect_to(sock_desc,client_id);
        switch (ret) {
          case 2:
                printf("%sErrore, utente non trovato! Selezionare un utente in lista!\n",KRED);
                printf("%s\n",KNRM);
                return;
          case 1: // client trovato e accetta, deve partire la chat end_end_chat
                return end_end_chat(sock_desc);
          case 0:
                printf("%sErrore, L'utente ha rifiutato la richiesta di connessione!\n",KRED);
                printf("%s\n",KNRM);
                return;
          default:// errore generico
                  return;
        }
    }else{
      printf("%sComando errato, inserire \"help\" per maggiori informazioni\n",KRED);
      printf("%s\n",KNRM);
    }
}


//Funzione eseguita dal thread di input
void* mini_shell(void* struttura){
  char command[BUF_LEN];
  input_struct* params = (input_struct*) struttura;
  int sock_desc = params->sock_desc ;

  printf("\nScrivi \"%s\" per aiuto\n",HELP);
  command_request(LIST,sock_desc);

  while (1/*da cambiare*/) {
    printf("Inserisci un comando: ");
    fgets(command,sizeof(command),stdin);
    printf("\n");
    command_request(command,sock_desc);
  }
}

//Funzione eseguita dal thread di output
void* server_output(void* struttura) {
  char output_buf[BUF_LEN];
  output_struct* params = (output_struct*) struttura;
  int sock_desc = params->sock_desc;
  int ret;

  while (1/*cambiare*/) {
    ret = recv_message(sock_desc,output_buf,BUF_LEN);
    ERROR_HELPER(ret,"Errore recv_message output_thread: ");
  }
  pthread_exit(&status);
}
