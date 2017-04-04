
  /* Armare il signale -- POTREBBE ESSERE DEPRECATA

  struct sigaction usr_action;
  sigset_t block_mask;

  sigfillset (&block_mask);
  usr_action.sa_handler = tactical_change;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  ret = sigaction (SIGUSR1, &usr_action, NULL);
  ERROR_HELPER(ret,"Errore armamento segnale SIGUSR1: ");
  */



    /*0660 utente del gruppo e proprietari hanno facoltà di modificarla e leggerla, gli altri no
    id_shared_memory = shmget(IPC_PRIVATE,CLIENT_SIZE*MAX_CLIENTS,IPC_CREAT|IPC_EXCL|0660);
    ERROR_HELPER(id_shared_memory,"Errore creazione shared memory: ");
    Forse deprecata.
    */


    //Verifica che l'utente user sia all'interno della shared memory,
    //se lo è allora lascia solo quell'utente in shared memory altrimenti l'area non viene toccata
    //ritorna 1 in caso vi sia, 0 in caso di utente non trovato
    /*int onList(char* user,int id_shared_memory){
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
    */



    /*gestore del segnale SIGUSR1 POTREBBE ESSERE DEPRECATA
    void tactical_change() {
      if(parent_status)
        parent_status = 0;
      else
        parent_status = 1;
    }
    */

    int parent_status = 1; // forse DEPRECATA


    //just 4 debug
    /*printf("Lancio una connessione verso:\n");
    printf("Nome: %s",target_user);
    printf("Porta: %s\n",port);
    printf("Indirizzo IP: ");
    printf("%s\n",ip );*/

    /*Forse deprecato
    ret = shmdt(pt);
    ERROR_HELPER(ret,"Errore scollegamento lettura CONNECT :");
    char* writer;
    writer = shmat(id_shared_memory,0,SHM_W);
    if(writer == (char*) -1)
      ERROR_HELPER(-1,"Errore agganciamento scrittura CONNECT : ");
    strncpy(writer,ip,strlen(list));
    strcat(writer,"\n");
    strncat(writer,port,strlen(port));
    strcat(writer,"\n");
    strncat(writer,target_user,strlen(target_user));
    ret = shmdt(writer);
    ERROR_HELPER(ret,"Errore scollegamento scrittura CONNECT: ");
    //invio segnale al processo forse deprecato
    kill(getppid(),SIGUSR1);

    */


    /*int connect_to(char* server,int port){
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
    }*/
/*
    int end_end_chat(int id_shared_memory){
      int ret;
      char* c;
      char* cour;
      char end_addr[MAX_IP_LEN];
      char end_name[MAX_LEN_NAME];
      char end_port[MAX_PORT_LEN];
      int socket;
      c = shmat(id_shared_memory, 0 , SHM_R);
      if ( c == (char*) -1 )
        ERROR_HELPER(-1,"Errore lettura memoria condivisa: ");
      //printf("Contenuto shared memory:\n");
      //printf("%s\n",c);
      cour = strtok(c,"\n");
      strcpy(end_addr,c);
      cour = strtok(NULL,"\n");
      strcpy(end_port,cour);
      cour = strtok(NULL,"\n");
      strcpy(end_name,cour);


      printf("Provo a connermi a %s all'indirizzo %s:%s\n",end_name,end_addr,end_port);

      socket = connect_to(end_addr,atoi(end_name));
      ERROR_HELPER(socket,"Errore connessione verso utente :");
      ret = shmdt(c);
      ERROR_HELPER(ret,"Errore scollegamento END2END: ");

      return socket;
    }*/


/*      char* p;
      p = shmat(id_shared_memory,0,SHM_W);
      if(p == (char*) -1)
        ERROR_HELPER(-1,"Errore agganciamento scrittura memoria condivisa: ");
      strncpy(p,list,strlen(list));
      ret = shmdt(p);
      ERROR_HELPER(ret,"Errore scollegamento LIST: ");
      */



      /*kill(getppid(),SIGINT);
      ret = shmctl(id_shared_memory,IPC_RMID,NULL);
      ERROR_HELPER(ret,"Errore cancellazione memoria condivisa: ");*/

/*
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
        }*/
