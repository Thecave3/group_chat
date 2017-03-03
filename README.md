# group_chat
This repo contains the project for "Sistemi Operativi" - Sapienza Univ. of Rome ay2016/2017

Officially we're coding only UNIX client, but maybe we'll create also the Windows client.

Server side is by Giorgio Dramis (che puzza).
Client side is by Andrea Lacava.


Here is the official request done by our teacher:

Talk

Realizzazione di un servizio "talk" via internet gestito tramite server.

Il servizio tiene traccia di un elenco di client pronti ad iniziare una
"conversazione".

I client (residenti, in generale, su macchine diverse), dopo essersi connessi
al servizio acquisiscono l'elenco e lo mostrano all'utente il quale potra'
collegarsi a un altro utente disponibile iniziando cosi' la conversazione.
Due client collegati tra loro permetteranno ai relativi utenti di scambiarsi
messaggi finche' uno dei due non interrompe la conversazione, e saranno
indisponibili ad altre conversazioni. Al termine di una conversazione i
client ritorneranno disponibili fino a che non si scollegano dal server.

Si precisa che la specifica richiede la realizzazione del software sia per
l'applicazione client che per l'applicazione server.

Per progetti misti Unix/Windows e' a scelta quale delle due applicazioni
sviluppare per uno dei due sistemi.

# server_linux

Funziona
* Registra ogni nuovo client che si connette al server
* Client a due stati {ONLINE, OFFLINE}
* Fornisce una lista dei client connessi
* Su richiesta del client lo rimuove dalla lista

In Lavorazione
* Logger delle operazioni sul server

Non ancora cominciato
* Semplificazione controllo del server
