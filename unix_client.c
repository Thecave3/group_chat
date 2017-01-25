#include "common.h"


int main(int argc, char const *argv[]) {
  int ret;

  ret= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (ret<0) ERROR_HELPER(ret,"Errore apertura socket");
  printf("OK, ret is: %d\n",ret);

  return 0;
}
