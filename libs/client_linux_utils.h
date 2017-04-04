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

void command_request(char* buffer,int sock_desc,) {
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

    }else if (strncmp(buffer,QUIT,strlen(QUIT))==0){
      printf("Chiusura connessione in corso... Bye Bye\n");
      server_disconnect(sock_desc);
      kill(getppid(),SIGINT);
      ret = shmctl(id_shared_memory,IPC_RMID,NULL);
      ERROR_HELPER(ret,"Errore cancellazione memoria condivisa: ");
      exit(EXIT_SUCCESS);
    }else if (strncmp(buffer,CLEAR,strlen(CLEAR))==0) {
      clear_screen();
    }else if (strncmp(buffer,CONNECT,strlen(CONNECT))==0) {
      user = subString(buffer,strlen(CONNECT)+1);
      printf("Hai scritto il comando connect verso %s",user);
      if(onList(user,id_shared_memory)){
        //utente trovato, qui la shared memory deve essere ripulita ed all'interno devono rimanere
        // nome utente, indirizzo IP e porta
        char* pt = shmat(id_shared_memory,0,SHM_R);
        if ( pt == (char*) -1 )
          ERROR_HELPER(-1,"Errore accesso shared memory: ");
        char* courier;
        char target_user[MAX_LEN_NAME];
        char port[MAX_PORT_LEN];
        char ip[MAX_IP_LEN];

        courier = strtok(pt,"\n");
        while (courier!= NULL) {
          strcpy(ip,courier);
          courier = strtok(NULL,"\n");
          strcpy(port,courier);
          courier=strtok(NULL,"\n");
          strcpy(target_user,courier);
          strcat(target_user,"\n");
          if(strcmp(target_user,user)==0)
            break;
          courier= strtok(NULL,"\n");
        }

        ret = connect_to(sock_desc,client_id);
        switch (ret) {
          case 2: //client non trovato
                break;
          case 1: // client trovato e accetta
                return end_end_chat(sock_desc);
          case 0:
                //conessione rifiutata
                break;
          default:// errore generico
                  break;
        }
        return;
      }else{
        printf("%sErrore, utente non trovato! Selezionare un utente in lista!\n",KRED);
        printf("%s\n",KNRM);
        return;
      }
    }else{
      printf("%sComando errato, inserire \"help\" per maggiori informazioni\n",KRED);
      printf("%s\n",KNRM);
    }
}


//Funzione che deve eseguire il thread di stdin
void* mini_shell(void* arg){
 printf("\nScrivi \"%s\" per aiuto\n",HELP);
 command_request(LIST,sock_desc);

 while (parent_status) {
   printf("Inserisci un comando: ");
   fgets(command,sizeof(command),stdin);
   printf("\n");
   command_request(command,sock);
 }
}
