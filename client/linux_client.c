#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"


int shouldStop = 0;

void* receiveMessage(void* arg) {
  int socket_desc = (int)(long)arg;
  char* close_command = QUIT;
  size_t close_command_len = strlen(close_command);

    /* select() uses sets of descriptors and a timeval interval. The
     * methods returns when either an event occurs on a descriptor in
     * the sets during the given interval, or when that time elapses.
     *
     * The first argument for select is the maximum descriptor among
     * those in the sets plus one. Note also that both the sets and
     * the timeval argument are modified by the call, so you should
     * reinitialize them across multiple invocations.
     *
     * On success, select() returns the number of descriptors in the
     * given sets for which data may be available, or 0 if the timeout
     * expires before any event occurs. */

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

        if (ret == 0) continue; // Viene scattato il timeout

        // at this point (ret==1) our message has been received!

        /**
        Codice gestione lettura chat
         **/
        int bytes_read = 0;

        while (1) {
          ret = read(socket_desc,buf+bytes_read,1);
          if (ret == -1) {
            if (errno == EINTR) {
              fprintf(stderr,"Errore lettura dati, ripeto \n");//mi serve davvero sta riga?
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

        // Gestione chiusura
        // (note that we subtract 1 to skip the message delimiter '\n')
        if (bytes_read - 1 == close_command_len && !memcmp(buf, close_command, close_command_len)) {
            fprintf(stderr, "Sessione di chat terminata dall'altro utente.\nPlease press ENTER to exit.\n");
            shouldStop = 1;
        } else {
          buf[bytes_read] = '\0';


          // Stampa messaggio
          printf("==> %s \n", buf);
        }
      }

    pthread_exit(NULL);
}

void* sendMessage(void* arg) {
    int socket_desc = (int)(long)arg;

    char* close_command = QUIT;
    size_t close_command_len = strlen(close_command);

    char buf[BUF_LEN];

    while (!shouldStop) {
        /* Read a line from stdin: fgets() reads up to sizeof(buf)-1
         * bytes and on success returns the first argument passed.
         * Note that '\n' is added at the end of the message when ENTER
         * is pressed: we can thus use it as our message delimiter! */
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        // Controlla se il server ha chiuso la connessione
        if (shouldStop){
          fprintf(stderr, "%sConnection closed by server\n",KRED );
          break;
        }

        // Numero di bytes da mandare (senza string terminator '\0')
        size_t msg_len = strlen(buf);

        /**
          Codice gestione scrittura chat
         **/
         int bytes_written =0;
         int ret;
         while (1) {
           ret = write(socket_desc,buf+bytes_written,msg_len);
           if (ret==-1) {
             if (errno == EINTR) {
               fprintf(stderr,"Errore scrittura dati, riperto\n");
               continue;
             }
             ERROR_HELPER(ret,"Errore scrittura dati fatale, panico \n");
           } else if ((bytes_written += ret) == msg_len) break;
         }


        // if we just sent a quit command, we need to update shouldStop!
        // (note that we subtract 1 to skip the message delimiter '\n')
        if (msg_len - 1 == close_command_len && !memcmp(buf, close_command, close_command_len)) {
            shouldStop = 1;
            fprintf(stderr, "Chat terminata! Bye Bye!\n");
        }
    }

    pthread_exit(NULL);
}


// Gestione CTRL-C
void kill_handler() {
  	//int ret;
    //cambio valore di shouldStop
    //invia stop ai due thread
    //server_disconnect() disconnette il server
    printf("Chiusura connessione effettuata, bye bye\n");
    exit(EXIT_SUCCESS);
  }

void init_threads(int socket_desc) {
    int ret;

    fprintf(stderr, "Connessione con il server avvenuta!\nDigita \"%s\" per uscire dal programma.\n", QUIT);

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
    ERROR_HELPER(socket_desc, "Errore creazione socket: ");

    // Set up dei parametri per a connessione
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);

    // Connessione alla socket del server
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    ERROR_HELPER(ret, "Errore connessione al server: ");

    // Invio username
    strcat(username,"\n");
    ret = send_message(socket_desc,username,strlen(username));
    ERROR_HELPER(ret,"Errore invio username: ");

    // Lancio inizializzazione shell
    init_threads(socket_desc);
}


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
        connectTo(argv[1]);
      }
    } else {
      syntaxError(argv[0]);
    }

    exit(EXIT_SUCCESS);
}
