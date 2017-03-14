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

int parent_status = 1;

//gestore del segnale SIGUSR1
void tactical_change() {
  if(parent_status)
    parent_status = 0;
  else
    parent_status = 1;
}


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
  ERROR_HELPER(ret,"Errore connessione: ");
  printf("Connessione riuscita\n");
  return sock;
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

//Verifica che l'utente user sia all'interno della shared memory,
//se lo è allora lascia solo quell'utente in shared memory altrimenti l'area non viene toccata
//ritorna 1 in caso vi sia, 0 in caso di utente non trovato
int onList(char* user,int id_shared_memory){
  //lettura memoria condivisa
  char* c;
  c = shmat(id_shared_memory, 0 , SHM_R);
  if ( c == (char*) -1 )
    ERROR_HELPER(-1,"Errore lettura memoria condivisa: ");
  char* candidate;
  for(int i = 0;c!=NULL;i++){
    if(c[i]==user[0]){
      candidate = (char*)malloc(strlen(user)*sizeof(char));
      for(int j=0;j<strlen(user);j++){
        if((i+j)>=strlen(c))
          return 0;
        candidate[j]=c[i+j];
      }
      if(strcmp(candidate,user) == 0){
        free(candidate);
        return 1;
      }
      free(candidate);
    }
  }
  return 0;
}


//apre e cerca all'interno della memoria condivisa il nome, l'IP address e la porta e instaura una connessione verso di esso
//restituisce il descrittore della connessione
int end_end_chat(int id_shared_memory){
  char* c;
  char* end_addr;
  char* end_name;
  //int end_port,socket;
  c = shmat(id_shared_memory, 0 , SHM_R);
  if ( c == (char*) -1 )
    ERROR_HELPER(-1,"Errore lettura memoria condivisa: ");
  printf("Contenuto shared memory:\n");
  printf("%s\n",c);


  //end_addr = parser(c,'\n');
  //end_name = parser(c+strlen(end_addr)+1,'\n');
  //end_port = (int) parser(c+strlen(end_addr)+strlen(end_name)+2,'\n');
  printf("Provo a connermi a %s all'indirizzo %s \n",end_name,end_addr);

  socket = connect_to(end_addr,end_name);
  ERROR_HELPER(socket,"Errore connessione verso utente :");
  return socket;

 }

void command_request(char* buffer,int sock_desc,int id_shared_memory) {
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

      char* p;
      p = shmat(id_shared_memory,0,SHM_W);
      if(p == (char*) -1)
        ERROR_HELPER(-1,"Errore scrittura memoria condivisa: ");
      strncpy(p,list,strlen(list));

    }else if (strncmp(buffer,QUIT,strlen(QUIT))==0){
      printf("Chiusura connessione in corso... Bye Bye\n");
      //todo nella fase finale dovrà inoltre inviare un segnale di chiusura a tutte le connessioni
      server_disconnect(sock_desc);
      ret = shmctl(id_shared_memory,IPC_RMID,NULL);
      ERROR_HELPER(ret,"Errore cancellazione memoria condivisa: ");
      exit(EXIT_SUCCESS);
    }else if (strncmp(buffer,CLEAR,strlen(CLEAR))==0) {
      clear_screen();
    }else if (strncmp(buffer,CONNECT,strlen(CONNECT))==0) {
      user = subString(buffer,strlen(CONNECT)+1);
      //printf("Hai scritto il comando connect verso %s",user);
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
        printf("Lancio una connessione verso:\n");
        printf("Nome: %s",target_user);
        printf("Porta: %s\n",port);
        printf("Indirizzo IP: ");
        printf("%s\n",ip );

        //prendere e fare una becerissima pulizia di memoria per permettere a end_end_chat di prendere target port e ip


        //invio segnale al processo
        kill(getppid(),SIGUSR1);

        //sarebbe opportuno prima di tornare bloccare un terminare ed aprirne un altro reindirizzandoci stdin e stdout
        return ;
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
