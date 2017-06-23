#include "../libs/client_linux_utils.h"


volatile sig_atomic_t shouldStop = 0;
volatile sig_atomic_t shouldWait = 0;
volatile sig_atomic_t onChat = 0;
volatile sig_atomic_t isRequest = 0;
long int socket_desc;

// Gestione CTRL-C
void kill_handler() {
  shouldStop = 1;
  char* close_command = QUIT;
  size_t close_command_len = strlen(close_command);
  int bytes_written = 0;
  int ret;
  while (1) {
    ret = write(socket_desc,close_command+bytes_written,close_command_len);
    if (ret==-1) {
      if (errno == EINTR) {
        fprintf(stderr,"Errore scrittura dati, ripeto\n");
        continue;
      }
      ERROR_HELPER(ret,"Errore scrittura dati fatale, panico");
    } else if ((bytes_written += ret) == close_command_len) break;
  }
  printf("Chiusura connessione effettuata, bye bye\n");
  exit(EXIT_SUCCESS);
}

// Routine di ricezione dei messaggi
void* receiveMessage() {
  char* close_command = QUIT;
  size_t close_command_len = strlen(close_command);
  char* request_command = CONNECT;
  size_t request_command_len = strlen(request_command);
  char* already_used_alert = NAME_ALREADY_USED;
  size_t already_used_alert_len = strlen(already_used_alert);
  char* connect_with_yourself= CONNECT_WITH_YOURSELF;
  size_t connect_with_yourself_len = strlen(connect_with_yourself);
  char* client_not_exist= CLIENT_NOT_EXIST;
  size_t client_not_exist_len = strlen(client_not_exist);
  char* list = LIST;
  size_t list_len = strlen(list);

  // Serve settare un intervallo per evitare di intasare la CPU con controlli e per controllare lo stato dell'altro thread
  struct timeval timeout;
  fd_set read_descriptors;
  int nfds = socket_desc + 1;

  char buf[BUF_LEN];

  // Codice gestione lettura chat
  int bytes_read;
  int ret;

  while (!shouldStop) {
    // Popolo i valori della struttura del timeout in modo da fare il check ogni 1.5 secondi in modo da lasciare la CPU il più libera possibile
    timeout.tv_sec  = 1;
    timeout.tv_usec = 500000;

    // Inizializzo a 0 i bit del set di file descriptors da passare alla select
    FD_ZERO(&read_descriptors);
    // Imposto nel set di file descriptors il valore del socket descriptor ottenuto dalla connessione del server
    FD_SET(socket_desc, &read_descriptors);

    // Lancio la select
    ret = select(nfds, &read_descriptors, NULL, NULL, &timeout);

    if (ret == -1 && errno == EINTR) continue;
    ERROR_HELPER(ret, "Errore select");

    if (ret == 0) continue; // Timeout scaduto

    // In questo momento (ret==1) quindi è stato ricevuto il messaggio
    bytes_read = 0;

    while (1) {
      ret = read(socket_desc,buf+bytes_read,1);
      if (ret == -1) {
        if (errno == EINTR) {
          continue;
        }else{
          ERROR_HELPER(ret,"Errore nella read, panico");
        }
      }
      if (ret == 0) {
        shouldStop = 1;
        break;
      }
      if(buf[bytes_read] == '\n') break;
      bytes_read++;
    }

    if (strncmp(buf,already_used_alert,already_used_alert_len)==0) { // Gestione name already used
      printf("\r%sErrore, nome già in uso sul server\n",KRED);
      shouldStop = 1;
      exit(EXIT_SUCCESS);
    } else if (strncmp(buf,list,list_len)==0) { // Gestione lista
      printf("\rLista utenti connessi:\n");
      shouldWait = 0;
    } else if (strncmp(buf,connect_with_yourself,connect_with_yourself_len)==0) { // Gestione connessione con utente non connesso
      printf("\r%sErrore, non puoi connetterti con te stesso%s\n",KRED,KNRM);
      shouldWait = 0;
    } else if (strncmp(buf,client_not_exist,client_not_exist_len)==0) { // Gestione connessione con se stessi
      printf("\r%sErrore, l'utente scelto non è connesso%s\n",KRED,KNRM);
      shouldWait = 0;
    } else if (strncmp(buf,request_command,request_command_len)==0) { // Gestione richiesta connessione
      shouldWait = 1;
      printf("\rHai una richiesta di connessione da parte di un altro utente!\n");
      printf("Rispondi %syes%s per accettare oppure %sno%s per rifiutare\n",KGRN,KNRM,KRED,KNRM);
      memset(buf, 0, BUF_LEN); // TODO testare senza questa riga dopo soluzione del segfault sul server
      onChat = 1;
      isRequest = 1;
      shouldWait = 0;
    }else if (strncmp(buf,close_command,close_command_len)==0) { // Gestione chiusura
      if(!onChat){
        fprintf(stderr, "Il server ha chiuso la connessione\n");
      }else{
        fprintf(stderr, "Sessione di chat terminata dall'altro utente.\n");
        onChat = 0;
      }
    } else { // Stampa messaggio
      buf[bytes_read] = '\0';
      bytes_read>0? printf("\r==> %s\n", buf) : printf("\r");;
      ret = fflush(stdout);
      ERROR_HELPER(ret,"Errore fflush");
    }
    printf(">> ");
    ret = fflush(stdout);
    ERROR_HELPER(ret,"Errore fflush");
  }
  pthread_exit(NULL);
}

// Routine di gestione dell'invio dei messaggi
void* sendMessage() {
  int ret,bytes_written;
  char buf[BUF_LEN];
  volatile sig_atomic_t shouldSend = 0;
  char* close_command = QUIT;
  size_t close_command_len = strlen(close_command);
  size_t msg_len;
  printf(">> ");
  ERROR_HELPER(fflush(stdout),"Errore fflush");
  while (!shouldStop) {
    if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
      fprintf(stderr, "%sErrore lettura input, uscita in corso...\n",KRED);
      exit(EXIT_FAILURE);
    }
    //  strcat(buf,"\n"); // non mi ricordo perchè facevamo sta cosa
    // Controlla se il server ha chiuso la connessione
    if (shouldStop){
      fprintf(stderr, "%sConnessione chiusa dal server\n",KRED );
      break;
    }

    if(!onChat){
      // Analizzo l'input utente interpretando i comandi
      if(strlen(buf)<MIN_CMD_LEN){
        printf("%sComando non riconosciuto, inserire \"%s\" per maggiori informazioni\n",KRED,HELP);
        printf("%s\n",KNRM);
        shouldSend = 0;
      } else if (strncmp(buf,CLEAR,strlen(CLEAR))==0) {
        clear_screen();
        shouldSend = 0;
      } else if (strncmp(buf,HELP,strlen(HELP))==0) {
        display_commands();
        shouldSend = 0;
      } else if (strncmp(buf,QUIT,strlen(QUIT))==0){
        printf("Chiusura connessione in corso...\n");
        shouldSend = 1;
      } else if (strncmp(buf,LIST,strlen(LIST))==0) {
        shouldSend = 1;
      } else if (strncmp(buf,CONNECT,strlen(CONNECT))==0) {
        char user[MAX_LEN_NAME];
        for(int i =0; i<MAX_LEN_NAME && i<strlen(buf);i++){
          user[i]=buf[strlen(CONNECT)+i];
        }
        if(strlen(user) <= 1 || strlen(user) > MAX_LEN_NAME){
          printf("%sInserisci un nome utente valido%s\n",KRED,KNRM);
          shouldSend = 0;
        }else{
          /*
          A questo punto l'utente scelto riceve dal server una richiesta di collegamento,
          l'utente che la ha istanziata deve rimanere in attesa e non può più inviare nulla
          al server finchè non c'è una risposta
          */
          printf("%sHai scritto il comando connect verso %s%s",KYEL,user,KNRM);
          printf("%sL'input è disabilitato fino alla risposta del server%s\n",KBLU,KNRM);
          shouldSend = 1;
          shouldWait = 1;
        }
      } else {
        printf("%sComando errato, inserire \"%s\" per maggiori informazioni%s\n",KRED,HELP,KNRM);
        shouldSend = 0;
      }
    } else if(isRequest) {
      // Se arriva qui significa che allora vi è stata una richiesta di chat.
      // Il ciclo while serve solo a controllare che la risposta al server sia YES oppure NO
      while (!(strncmp(buf,YES,strlen(YES)) == 0 || strncmp(buf,NO,strlen(NO)) == 0) && isRequest){
        printf("Rispondi %syes%s per accettare oppure %sno%s per rifiutare\n",KGRN,KNRM,KRED,KNRM);
        printf(">> ");
        ret = fflush(stdout);
        ERROR_HELPER(ret,"Errore fflush");
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
          fprintf(stderr, "%sErrore lettura input, uscita in corso...\n",KRED);
          exit(EXIT_FAILURE);
        }
      }
      onChat = strncmp(buf,YES,strlen(YES)) == 0?1:0;
      shouldSend = 1;
      isRequest = 0;
      // Inizia la chat, il client invia uno yes al server,non devo più interpretare i comandi tranne il quit
      // Nessuna chat, il client invia una risposta di no al server e torna la shell
    } else {

    }

    if(shouldSend){
      // Codice gestione invio dati server
      bytes_written = 0;
      // Numero di bytes da mandare (senza string terminator '\0')
      msg_len = strlen(buf);

      while (1) {
        ret = write(socket_desc,buf+bytes_written,msg_len);
        if (ret==-1) {
          if (errno == EINTR) {
            fprintf(stderr,"Errore scrittura dati, ripeto\n");
            continue;
          }
          ERROR_HELPER(ret,"Errore scrittura dati fatale, panico");
        } else if ((bytes_written += ret) == msg_len) break;
      }

      // È stato inviato il comando QUIT, bisogna chiudere la chat oppure il programma aggiornando shouldStop e onChat
      if (strncmp(buf,close_command,close_command_len)==0) {
        if (onChat) {
          onChat = 0;
        }else{
          shouldStop = 1;
          exit(EXIT_SUCCESS);
        }
      }
    }

    while (shouldWait) {
      sleep(1);
    }
  }
  pthread_exit(NULL);
}

void init_threads() {
  int ret;

  printf("Connessione con il server avvenuta!\n");

  pthread_t chat_threads[2];

  ret = pthread_create(&chat_threads[0], NULL, receiveMessage, NULL);
  PTHREAD_ERROR_HELPER(ret, "Errore creazione thread ricezione messaggi");

  ret = pthread_create(&chat_threads[1], NULL, sendMessage, NULL);
  PTHREAD_ERROR_HELPER(ret, "Errore creazione thread invio messaggi");

  // Armo il segnale per gestire il CTRL-C
  signal(SIGINT,kill_handler);

  // Aspetto la terminazione del programma
  ret = pthread_join(chat_threads[0], NULL);
  PTHREAD_ERROR_HELPER(ret, "Errore join thread ricezione messaggi");

  ret = pthread_join(chat_threads[1], NULL);
  PTHREAD_ERROR_HELPER(ret, "Errore join thread invio messaggi");

  // Chiusura socket
  ret = close(socket_desc);
  ERROR_HELPER(ret, "Errore chiusura socket");
}


// Effettua una connessione TCP con il server inviando il nome
void connectTo(char* username) {
  int ret;
  struct sockaddr_in server_addr = {0};

  // Creazione socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  ERROR_HELPER(socket_desc, "Errore creazione socket");

  // Set up dei parametri per a connessione
  server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
  server_addr.sin_family      = AF_INET;
  server_addr.sin_port        = htons(SERVER_PORT);

  // Connessione alla socket del server
  ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
  ERROR_HELPER(ret, "Errore connessione al server");

  // Invio username
  strcat(username,"\n");
  ret = send_message(socket_desc,username,strlen(username));
  ERROR_HELPER(ret,"Errore invio username");

  // Lancio inizializzazione shell
  init_threads();
}

// Gestisce input errati da parte dell'utente all'inizio del programma
void syntaxError(char* prog_name) {
  fprintf(stderr, "Uso del programma:\n");
  fprintf(stderr, "       %s <nome_utente>\n", prog_name);
  fprintf(stderr, "Nota che <nome_utente> deve essere al massimo di %d caratteri\n", MAX_LEN_NAME);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    // Prendo il nome utente
    if (strlen(argv[1])>MAX_LEN_NAME){
      fprintf(stderr,"%sNome inserito troppo lungo!\n%s",KRED,KNRM);
      syntaxError(argv[0]);
    } else {
      printf("Benvenuto %s\n", argv[1]);
      printf("Provo a connettermi al server...\n");
      connectTo(argv[1]);
    }
  } else {
    syntaxError(argv[0]);
  }

  exit(EXIT_SUCCESS);
}
