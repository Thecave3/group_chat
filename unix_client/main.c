#include "../common.h"


int main(int argc, char const *argv[]) {
  int ret; //variabile sentinella per i valori
  char* msg = malloc(sizeof(char)*ERR_MSG_LENGTH);
  ret=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (ret<0) ERROR_HELPER(ret,msg);



    printf("OK, ret is: %d\n",ret);
  return 0;
}
