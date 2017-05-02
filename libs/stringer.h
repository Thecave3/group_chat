#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//subString:
//Data una stringa string e un indice index, estrae e ritorna la sottostringa che parte da string[index]
//Ritorna "" in caso di errore;
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
