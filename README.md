# group_chat
This repo contains the project for "Sistemi Operativi" - Sapienza Univ. of Rome ay2016/2017

Officially we're coding only UNIX client, but maybe we'll create also the Windows client.

* Server side is by Giorgio Dramis.
* Client side is by Andrea Lacava.


Here is the official request done by our teacher:

Talk

Realizzazione di un servizio "talk" via internet gestito tramite server.

Il servizio tiene traccia di un elenco di client pronti ad iniziare una
"conversazione".

I client (residenti, in generale, su macchine diverse), dopo essersi connessi
al servizio acquisiscono l'elenco e lo mostrano all'utente il quale potrà
collegarsi a un altro utente disponibile iniziando cosi' la conversazione.
Due client collegati tra loro permetteranno ai relativi utenti di scambiarsi
messaggi finche' uno dei due non interrompe la conversazione, e saranno
indisponibili ad altre conversazioni. Al termine di una conversazione i
client ritorneranno disponibili fino a che non si scollegano dal server.

Si precisa che la specifica richiede la realizzazione del software sia per
l'applicazione client che per l'applicazione server.

Per progetti misti Unix/Windows e' a scelta quale delle due applicazioni
sviluppare per uno dei due sistemi.

## server_linux

Architecture concept:
* Server multiprocesso e multithread:
  - Il processo padre, a seconda del comando passatogli, genera un demone e una cartella nella home dell'utente per i file di log. Se avviato con un comando diverso termina il demone.
  - Il demone si occupa di accettare nuove connessioni in ingresso, se un client si connette viene immediatamente predisposto un thread per gestire quest'ultimo.
  - Ogni thread si occupa del setup e della gestione dei comandi del client ad esso associato, nel caso di una eventuale connessione da parte di un client da o verso un altro client, dopo l'inoltro della richiesta di connessione, il thread associato reindirizzerà i messaggi a quest'ultimo e viceversa fino all'invio del messaggio "quit" da parte di uno dei due client.
* Le strutture dati del server sono:
  - <code>client_list</code>: una SCL in cui vengono registrati tutti i client connessi che hanno superato il processo di riconoscimento.  Questa struttura è accessibile ad ogni thread, e ne permette la comunicazione e/o la sincronizzazione. La mutua esclusione dei dati contenuti al suo interno in caso di modifica è garantita da un semaforo binario per la SCL in se, e un altro semaforo binario per ogni struct all'interno di quest'ultima.
  - <code>thread_list</code>: una SCL che memorizza gli handler di ogni thread e i relativi argomenti. Non necessita di mutua esclusione in quanto ultilizzata esclusivamente dal processo figlio e non dai thread.
* Funzionalità aggiuntive:
  - Garbage collector: ogni TIMEX secondi il processo padre richiama la funzione <code>void garbage_collector (int ignored)</code>. Tale funzione scorre la thread_list e tramite un flag individua i thread inattivi o estinti ma le cui strutture dati sono ancora intatte e a seconda dei casi li estingue e/o libera la memoria occupata da tali strutture.
  - Logger: all'interno della cartella precedentemente creata dal processo padre vengono creati un file di log per il processo figlio e, all'interno di una sottocartella, un logfile separato per ogni client connesso. Dato il particolare schema di interconnessione tali log devono poter essere scritti in concorrenza, pertanto per ogni logfile è presente un semaforo binario che ne garantisce la mutua esclusione.

Chiamate disponibili per l'utente:
* <code>--start</code> Avvia il demone server.
* <code>--kill</code> Termina il demone server.

## client_linux

Architecture concept:
* Client Multithread e shell utente che gestisce gli user-input e manda le custom-requests al server
* Interazione con il server tramite custom API integrate nella shell poggianti su chiamate C-UNIX
* Gestione di segnali come SIGINT e SIGPIPE:
  - SIGINT: invia al server un segnale di chiusura client e termina il programma.
  - SIGPIPE: termina direttamente la chat killando i thread.
* Rappresentazione degli stati del client tramite variabili atomiche che quindi garantiscono la consistenza degli stati evitando momenti di stati non definiti in caso di scheduling da parte della CPU.

Chiamate disponibili per l'utente:

* <code>connect USER</code> : inoltra una richiesta di chat verso l'utente identificato con il nome USER
* <code>list</code> : esegue il download della lista aggiornata di utenti connessi al server
* <code>clear</code> : pulisce lo schermo
* <code>quit</code> : invia un segnale di disconnessione al server e chiude il client
* <code>help</code> : invoca una stampa dei comandi disponibili

NB: il client non mantiene alcuna informazione sullo stato degli altri client connessi (dumb client), il server carica e gestisce ogni interazione tra i vari client.

Known issue del client: la gestione grafica della list, ma è un bug prettamente dovuto all'uso di un unico terminale per input ed output.
