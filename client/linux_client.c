#include "../libs/client_linux_utils.h"


volatile sig_atomic_t shouldStop = 0;
volatile sig_atomic_t shouldWait = 0;
volatile sig_atomic_t onChat = 0;

// Gestione CTRL-C
void kill_handler() {
  shouldStop = 1;
  printf("Chiusura connessione effettuata, bye bye\n");
  exit(EXIT_SUCCESS);
}

// Routine di ricezione dei messaggi
void* receiveMessage(void* arg) {
  int socket_desc = (int)(long)arg;
  char* close_command = QUIT;
  size_t close_command_len = strlen(close_command);
  char* request_command = CONNECT ;
  size_t request_command_len = strlen(request_command);
  char* already_used_alert = NAME_ALREADY_USED ;
  size_t already_used_alert_len = strlen(already_used_alert);

  // Serve settare un intervallo per evitare di intasare la CPU con controlli
  struct timeval timeout;
  fd_set read_descriptors;
  int nfds = socket_desc + 1;

  char buf[BUF_LEN];

  while (!shouldStop) {
    int ret;

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
    ERROR_HELPER(ret, "Errore select!");

    if (ret == 0) continue; // Timeout scaduto

    // In questo momento (ret==1) quindi è stato ricevuto il messaggio

    // Codice gestione lettura chat

    int bytes_read = 0;

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

    // Gestione name already used
    if (strncmp(buf,already_used_alert,already_used_alert_len)==0) {
      printf("\n%sErrore, nome già in uso sul server\n",KRED);
      shouldStop = 1;
      kill_handler();
    }

    // Gestione richiesta connessione
    if (strncmp(buf,request_command,request_command_len)==0) {
      printf("Hai una richiesta di connessione da parte di un altro utente!\n");
      printf("Rispondi %syes%s per accettare oppure %sno%s per rifiutare\n",KGRN,KNRM,KRED,KNRM);
      if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
        fprintf(stderr, "%sErrore lettura input, uscita in corso...\n",KRED);
        exit(EXIT_FAILURE);
      }
      while (strncmp(buf,"yes",strlen("yes")) == 0 || strncmp(buf,"no",strlen("no")) == 0) {
        printf("%sErrore%s\n",KRED,KNRM);
        printf("Rispondi %syes%s per accettare oppure %sno%s per rifiutare\n",KGRN,KNRM,KRED,KNRM);
        printf(">> ");
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
          fprintf(stderr, "%sErrore lettura input, uscita in corso...\n",KRED);
          exit(EXIT_FAILURE);
        }
      }
      shouldWait = 0;
    }

    // Gestione chiusura
    if (strncmp(buf,close_command,close_command_len)==0) {
      fprintf(stderr, "Sessione di chat terminata dall'altro utente.\nPremi ENTER per uscire.\n");
      shouldStop = 1;
    } else {
      buf[bytes_read] = '\0';

      // Stampa messaggio
      printf("\n==> %s \n", buf);
      ret = fflush(stdout);
      ERROR_HELPER(ret,"Errore fflush");
    }
  }

  pthread_exit(NULL);
}

// Routine di gestione dell'invio dei messaggi
void* sendMessage(void* arg) {
  int socket_desc = (int)(long)arg;
  int ret;
  char buf[BUF_LEN];

  char* close_command = QUIT;
  size_t close_command_len = strlen(close_command);
  volatile sig_atomic_t shouldSend = 0;


  while (!shouldStop) {
    printf("\n>> ");
    if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
        fprintf(stderr, "%sErrore lettura input, uscita in corso...\n",KRED);
        exit(EXIT_FAILURE);
      }
    strcat(buf,"\n");
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
        printf("Lista utenti connessi:\n");
        strncpy(buf,"list\n",strlen(buf));
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
          printf("%sHai scritto il comando connect verso %s%s\n",KYEL,user,KNRM);
          printf("%sL'input è disabilitato fino alla risposta del server%s\n",KBLU,KNRM);
          shouldSend = 1;
          shouldWait = 1;
        }
      /*
      A questo punto l'utente scelto riceve dal server una richiesta di collegamento,
      l'utente che la hai istanziata deve rimanere in attesa e non può più inviare nulla
      al server finchè non c'è una risposta
      */
    } else {
      printf("%sComando errato, inserire \"%s\" per maggiori informazioni\n",KRED,HELP);
      printf("%s\n",KNRM);
      shouldSend = 0;
    }
  }

    if(shouldSend){
      // Numero di bytes da mandare (senza string terminator '\0')
      // Codice gestione invio dati server
      size_t msg_len = strlen(buf);
      int bytes_written = 0;

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
      // (note that we subtract 1 to skip the message delimiter '\n')
      if (strncmp(buf,close_command,close_command_len)==0) {
        if (onChat) {
          onChat = 0;
        }else{
          shouldStop = 1;
          kill_handler();
        }
      }
    }

    while (shouldWait) {
      sleep(1000);
    }
  }
  pthread_exit(NULL);
}

void init_threads(int socket_desc) {
  int ret;

  fprintf(stderr, "\nConnessione con il server avvenuta!\n");

  pthread_t chat_threads[2];

  ret = pthread_create(&chat_threads[0], NULL, receiveMessage, (void*)(long)socket_desc);
  PTHREAD_ERROR_HELPER(ret, "Errore creazione thread ricezione messaggi");

  ret = pthread_create(&chat_threads[1], NULL, sendMessage, (void*)(long)socket_desc);
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
  int socket_desc;
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
  init_threads(socket_desc);
}

// Gestisce input errati da parte dell'utente all'inizio del programma
void syntaxError(char* prog_name) {
  fprintf(stderr, "Uso del programma:\n\n");
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
