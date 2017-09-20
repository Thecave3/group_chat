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

* Mantiene una lista dei client connessi e del loro stato {ONLINE, OFFLINE}
* Restituisce una lista di client connessi con stato ONLINE
* Aggiunto un log del server e delle interazioni con i client su file per il monitoraggio

## client_linux

Architecture concept:
* Client Multithread e shell utente che gestisce gli user-input e manda le custom-requests al server
* Interazione con il server tramite custom API integrate nella shell poggianti su chiamate C-UNIX
* Gestione di segnali come SIGINT e SIGPIPE:
 - SIGINT: invia al server un segnale di chiusura client e termina il programma.
 - SIGPIPE: termina direttamente la chat killando i thread.
* Rappresentazione degli stati del client tramite variabili atomiche che quindi garantiscono la consistenza degli stati evitando momenti di stati non definiti in caso di scheduling da parte della CPU.

Chiamate disponibili per l'utente:

* connect USER : inoltra una richiesta di chat verso l'utente identificato con il nome USER
* list : esegue il download della lista aggiornata di utenti connessi al server
* clear : pulisce lo schermo
* quit : invia un segnale di disconnessione al server e chiude il client
* help : invoca una stampa dei comandi disponibili

NB: il client non mantiene alcuna informazione sullo stato degli altri client connessi (dumb client), il server carica e gestisce ogni interazione tra i vari client.

Known issue del client: la gestione grafica della list, ma è un bug prettamente dovuto all'uso di un unico terminale per input ed output.
