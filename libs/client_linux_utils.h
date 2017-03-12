#include "colors.h"
#include "stringer.h"

#include "../libs/server_protocol.h"
#include "../libs/common.h"

#define DEFAULT_NAME "thecave3"
#define LIST "list"
#define CONNECT "connect"
#define HELP "help"
#define CLEAR "clear"
#define QUIT "quit"

#define EXIT_SUCCESS 0

#define MIN_CMD_LEN 4


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


int connect_to(char* server,int port){
  int ret;
  int sock = socket(AF_INET,SOCK_STREAM,0);
  struct sockaddr_in server_addr = {0};
  ERROR_HELPER(sock,"Errore creazione socket: ");
  server_addr.sin_addr.s_addr = inet_addr(server);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  ret = connect(sock,(struct sockaddr*) &server_addr,sizeof(struct sockaddr_in));
  ERROR_HELPER(ret,"Errore connect: ");
  return sock;
}

void display_commands() {
  printf("-----------------LISTA COMANDI---------------\n\n");
  printf("%s",LIST);
  printf(":Mostra la lista degli utenti connessi\n\n");
  printf("%s",CONNECT);
  printf(" NOME_UTENTE: Richiesta connessione verso un utente \n\n");
  printf("%s",QUIT);
  printf(": Disconnessione dal server ed uscita dal programma.\n\n");
  printf("%s",CLEAR);
  printf(": Pulisci schermo\n\n");
  printf("%s",HELP);
  printf(": Mostra questa lista\n\n");
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
      printf("list\n");
      ret = download_list(sock_desc,list, sizeof(list));
      if (ret <= 0) exit(EXIT_FAILURE);
      printf("%s\n",list);
    }else if (strncmp(buffer,QUIT,strlen(QUIT))==0){
      printf("bye bye\n");
      exit(EXIT_SUCCESS);
    }else if (strncmp(buffer,CLEAR,strlen(CLEAR))==0) {
      clear_screen();
    }else if (strncmp(buffer,CONNECT,strlen(CONNECT))==0) {
        user = subString(buffer,strlen(CONNECT)+1);
        printf("Hai scritto il comando connect verso %s\n",user);
    }else{
      printf("%sComando errato, inserire \"help\" per maggiori informazioni\n",KRED);
      printf("%s\n",KNRM);
    }
}
