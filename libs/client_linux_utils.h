#ifndef CLIENT_LINUX_H
#define CLIENT_LINUX_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>


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

//Struttura per i parametri del thread di output
typedef struct{
  int sock_desc;
  sem_t semid;
  } output_struct;

//Pulisce lo schermo
void clear_screen() {
    printf("%s\e[1;1H\e[2J\n",KNRM);
}


//Mostra l'elenco completo dei comandi disponibile per l'utente
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


void* server_output(void* struttura) {
  char output_buf[BUF_LEN];
  output_struct* params = (output_struct*) struttura;
  int sock_desc = params->sock_desc;
  sem_t mutex_sem_stdout = params->semid;
  int ret;

  sem_wait(&mutex_sem_stdout);
  while (1) {
    ret = recv_message(sock_desc,output_buf,BUF_LEN);
    ERROR_HELPER(ret,"Errore recv_message output_thread: ");
    if (ret==0)
      printf("L'utente ha chiuso la chat.\n");
    printf("%s\n",output_buf);
  }
  sem_post(&mutex_sem_stdout);

  return NULL;
}



// Gestisce l'invio di messaggi tra due client via server
// Valori di ritorno:
// 0 in caso di uscita chat e ritorno applicazione
// 1 in caso di chiusura totale programma
int end_end_chat(int sock_desc){
  int ret;
  char input_buf[BUF_LEN];

  printf("Chat iniziata, digitare \"quit\" per chiudere la connessione.");

  do{
    fgets(input_buf,sizeof(input_buf),stdin);
    ret = send_message(sock_desc,input_buf,BUF_LEN);
    ERROR_HELPER(ret,"Errore send_message: ");
  }while(strncmp(input_buf,QUIT,strlen(QUIT))!=0);

  return 0;
}


// Gestisce la richiesta dei comandi disponibili per l'utente
// Valori di ritorno:
// 0 in caso di uscita terminata programmata dal flusso di esecuzione
// 1 in caso di chiusura
int command_request(char* buffer,int sock_desc,char* list) {
  char* user;
  int ret;
  for(int i =0; i<MIN_CMD_LEN;i++){
    if(buffer[i]=='\0'){
      printf("%sComando errato, inserire \"help\" per maggiori informazioni\n",KRED);
      printf("%s\n",KNRM);
      return 0;
      }
    }

    if (strncmp(buffer,HELP,strlen(HELP))==0) {
      display_commands();
      return 0;
    }else if (strncmp(buffer,LIST,strlen(LIST))==0) {
      download_list(sock_desc,list, sizeof(list));
      printf("Lista utenti connessi:\n%s\n",list);
      return 0;
    }else if (strncmp(buffer,QUIT,strlen(QUIT))==0){
      printf("Chiusura connessione in corso... Bye Bye\n");
      server_disconnect(sock_desc);
      return 1;
    }else if (strncmp(buffer,CLEAR,strlen(CLEAR))==0) {
      clear_screen();
      return 0;
    }else if (strncmp(buffer,CONNECT,strlen(CONNECT))==0) {
      user = subString(buffer,strlen(CONNECT)+1);
      printf("Hai scritto il comando connect verso %s",user);
      printf("Cerco utente .");
      //trovare il client_id all'interno di list
      int client_id = -1;
      for(int i = 0;i< sizeof(list)/sizeof(list[0]);i++){
        printf(" . ");
        if (strncmp(list+i,user,strlen(user))==0) {
          printf("Utente trovato!\n");
          client_id = list[i-2];
          break;
        }
      }

      if (client_id == -1) {
        printf("%sUtente non trovato!",KRED);
        printf("%s\n",KNRM);
        return 0;
      }

      ret = connect_to(sock_desc,client_id);
        switch (ret) {
          case 2:
                printf("%sErrore, utente non trovato! Selezionare un utente in lista!\n",KRED);
                printf("%s\n",KNRM);
                return 0;
          case 1: // client trovato e accetta, deve partire la chat end_end_chat
                return end_end_chat(sock_desc);
          case 0:
                printf("%sErrore, L'utente ha rifiutato la richiesta di connessione!\n",KRED);
                printf("%s\n",KNRM);
                return 0;
          default:// errore generico
                  return 0;
        }
    }else{
      printf("%sComando errato, inserire \"help\" per maggiori informazioni\n",KRED);
      printf("%s\n",KNRM);
      return 0;
    }
}
#endif
