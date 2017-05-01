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
