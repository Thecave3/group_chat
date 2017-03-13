#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//estrae la sottostringa ad index
char* subString(char* string,int index){
  if (index>strlen(string))
    return "";
  char* ret = (char*)malloc(sizeof(string));
  int i = index;
  int a = 0;
  do{
    ret[a]=string[i];
    a++;
    i++;
  }while(string[i]!= '\0');
  return ret;
}

//data una stringa string ed un carattere token, restituisce la prima sottostringa di string dall'inizio al carattere token (escluso)
//nb se non token non c'è nella stringa restituisce tutta la stringa
char* parser(char* string, char token){
  char* ret = (char*)malloc(sizeof(string));
  for(int i=0;string[i]!=token || string+i==NULL;i++){
    ret[i]=string[i];
  }
  return ret;
}
